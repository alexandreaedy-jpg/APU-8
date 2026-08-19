import os
import time
from typing import Dict, List, Optional, Tuple

try:
    import mido
except Exception:
    mido = None


STATE_FILE = r"C:\Users\mto1\Documents\NES_DEV\state_separate_channels.bin"
TMP_FILE = STATE_FILE + ".tmp"

MIDI_INPUT_NAME_HINT: Optional[str] = "Springbeats vMIDI1 0"
VERBOSE_EVENTS = False
PRINT_IGNORED_CC = False
FLUSH_INTERVAL_S = 1.0 / 120.0

CHANNEL_P1 = 12
CHANNEL_P2 = 13
CHANNEL_TRI = 14
CHANNEL_NOISE = 15
CHANNEL_GLOBAL = 16

ARP_OFF = 0
ARP_UP = 1
ARP_DOWN = 2
ARP_UPDOWN = 3


def clamp(v: int, lo: int, hi: int) -> int:
    return max(lo, min(hi, int(v)))


state = {
    "seq": 0,
    "attack": 8,
    "decay": 32,
    "sustain": 96,
    "release": 24,
}


voices: Dict[str, Dict[str, int]] = {
    "p1": {"note": 60, "vel": 100, "gate": 0},
    "p2": {"note": 64, "vel": 100, "gate": 0},
    "tri": {"note": 48, "vel": 100, "gate": 0},
    "noi": {"note": 36, "vel": 100, "gate": 0},
}

voice_note_stacks: Dict[str, List[Tuple[int, int]]] = {
    "p1": [],
    "p2": [],
    "tri": [],
    "noi": [],
}

global_note_stack: List[Tuple[int, int]] = []
last_payload: Optional[bytes] = None
state_dirty = False
last_flush_time = 0.0

arp_enabled = False
arp_mode = ARP_UP
arp_speed_cc = 32
arp_phase = 0
arp_dir = 1
last_arp_time = 0.0
arp_latch = False
arp_octaves = 0

voice_params: Dict[str, Dict[str, int]] = {
    "p1": {"duty": 2, "glide": 0, "vib_depth": 0, "vib_rate": 0, "attack": 8, "decay": 32, "sustain": 96, "release": 24},
    "p2": {"duty": 2, "glide": 0, "vib_depth": 0, "vib_rate": 0, "attack": 8, "decay": 32, "sustain": 96, "release": 24},
    "tri": {"glide": 0, "vib_depth": 0, "vib_rate": 0, "attack": 8, "decay": 32, "sustain": 96, "release": 24},
    "noi": {"attack": 8, "decay": 32, "sustain": 96, "release": 24, "timbre": 0, "mode": 0},
}


CHANNEL_TO_VOICE = {
    CHANNEL_P1: "p1",
    CHANNEL_P2: "p2",
    CHANNEL_TRI: "tri",
    CHANNEL_NOISE: "noi",
}


def bump_seq() -> None:
    state["seq"] = (state["seq"] + 1) & 0xFF


def print_state_summary() -> None:
    print("Etat initial :")
    print("  P1     : duty={duty} glide={glide} vib={vib_depth}/{vib_rate} adsr={attack}/{decay}/{sustain}/{release}".format(**voice_params["p1"]))
    print("  P2     : duty={duty} glide={glide} vib={vib_depth}/{vib_rate} adsr={attack}/{decay}/{sustain}/{release}".format(**voice_params["p2"]))
    print("  TRI    : glide={glide} vib={vib_depth}/{vib_rate} adsr={attack}/{decay}/{sustain}/{release}".format(**voice_params["tri"]))
    print("  NOI    : adsr={attack}/{decay}/{sustain}/{release} timbre={timbre} mode={mode}".format(**voice_params["noi"]))
    print("Canaux directs :")
    print(f"  CH{CHANNEL_P1} -> P1")
    print(f"  CH{CHANNEL_P2} -> P2")
    print(f"  CH{CHANNEL_TRI} -> TRI")
    print(f"  CH{CHANNEL_NOISE} -> NOISE")
    print(f"  CH{CHANNEL_GLOBAL} -> global chord split")
    print(f"  ARP CH16: {'ON' if arp_enabled else 'OFF'} mode={arp_mode} speed={arp_speed_cc} latch={'ON' if arp_latch else 'OFF'} oct={arp_octaves}")


def normalize_stack(stack: List[Tuple[int, int]]) -> List[Tuple[int, int]]:
    cleaned: List[Tuple[int, int]] = []
    seen = set()

    for note, vel in stack:
        note = clamp(note, 0, 127)
        vel = clamp(vel, 1, 127)
        if note in seen:
            continue
        seen.add(note)
        cleaned.append((note, vel))

    return cleaned


def build_payload() -> bytes:
    payload = [
        state["seq"],
        state["attack"],
        state["decay"],
        state["sustain"],
        state["release"],
        0,
        0,
        0,
        0,
        0,
        voices["p1"]["note"],
        voices["p1"]["vel"],
        voices["p1"]["gate"],
        voices["p2"]["note"],
        voices["p2"]["vel"],
        voices["p2"]["gate"],
        voices["tri"]["note"],
        voices["tri"]["vel"],
        voices["tri"]["gate"],
        voices["noi"]["note"],
        voices["noi"]["vel"],
        voices["noi"]["gate"],
        voice_params["p1"]["duty"],
        voice_params["p1"]["glide"],
        voice_params["p1"]["vib_depth"],
        voice_params["p1"]["vib_rate"],
        voice_params["p2"]["duty"],
        voice_params["p2"]["glide"],
        voice_params["p2"]["vib_depth"],
        voice_params["p2"]["vib_rate"],
        voice_params["tri"]["glide"],
        voice_params["tri"]["vib_depth"],
        voice_params["tri"]["vib_rate"],
        voice_params["p1"]["attack"],
        voice_params["p1"]["decay"],
        voice_params["p1"]["sustain"],
        voice_params["p1"]["release"],
        voice_params["p2"]["attack"],
        voice_params["p2"]["decay"],
        voice_params["p2"]["sustain"],
        voice_params["p2"]["release"],
        voice_params["tri"]["attack"],
        voice_params["tri"]["decay"],
        voice_params["tri"]["sustain"],
        voice_params["tri"]["release"],
        voice_params["noi"]["attack"],
        voice_params["noi"]["decay"],
        voice_params["noi"]["sustain"],
        voice_params["noi"]["release"],
        voice_params["noi"]["timbre"],
        voice_params["noi"]["mode"],
    ]

    while len(payload) < 58:
        payload.append(0)

    return bytes(payload)


def write_state() -> None:
    global last_payload, state_dirty, last_flush_time
    payload = build_payload()

    if payload == last_payload:
        state_dirty = False
        return

    with open(TMP_FILE, "wb") as f:
        f.write(payload)
        f.flush()

    try:
        os.replace(TMP_FILE, STATE_FILE)
    except PermissionError:
        with open(STATE_FILE, "wb") as f:
            f.write(payload)
            f.flush()
        try:
            os.remove(TMP_FILE)
        except OSError:
            pass

    last_payload = payload
    state_dirty = False
    last_flush_time = time.monotonic()


def mark_dirty() -> None:
    global state_dirty
    state_dirty = True


def flush_state_if_due(force: bool = False) -> None:
    if not state_dirty:
        return

    if force or (time.monotonic() - last_flush_time) >= FLUSH_INTERVAL_S:
        write_state()


def set_voice_note(voice_name: str, note: int, velocity: int, gate: int) -> None:
    voice = voices[voice_name]
    voice["note"] = clamp(note, 0, 127)
    voice["vel"] = clamp(velocity, 0, 127)
    voice["gate"] = 1 if gate else 0
    bump_seq()
    mark_dirty()
    if VERBOSE_EVENTS:
        print(f"{voice_name.upper()} -> note={voice['note']} vel={voice['vel']} gate={voice['gate']}")


def set_voice_stack(voice_name: str, stack: List[Tuple[int, int]]) -> None:
    voice_note_stacks[voice_name] = normalize_stack(stack)
    if voice_note_stacks[voice_name]:
        note, vel = voice_note_stacks[voice_name][0]
        set_voice_note(voice_name, note, vel, 1)
    else:
        voice = voices[voice_name]
        set_voice_note(voice_name, voice["note"], voice["vel"], 0)


def refresh_global_split() -> None:
    global global_note_stack, arp_phase
    global_note_stack = normalize_stack(global_note_stack)

    if arp_enabled and len(global_note_stack) > 0:
        if arp_phase >= len(global_note_stack):
            arp_phase = 0
        n0, v0 = global_note_stack[arp_phase]
        if arp_octaves > 0 and len(global_note_stack) > 0:
            octave_index = (arp_phase // len(global_note_stack)) % (arp_octaves + 1)
            n0 = clamp(n0 + (12 * octave_index), 0, 127)
        voices["p1"]["note"] = n0
        voices["p1"]["vel"] = v0
        voices["p1"]["gate"] = 1
        voices["p2"]["gate"] = 0
        voices["tri"]["gate"] = 0
    else:
        if len(global_note_stack) > 0:
            n0, v0 = global_note_stack[0]
            voices["p1"]["note"] = n0
            voices["p1"]["vel"] = v0
            voices["p1"]["gate"] = 1
        else:
            voices["p1"]["gate"] = 0

        if len(global_note_stack) > 1:
            n1, v1 = global_note_stack[1]
            voices["p2"]["note"] = n1
            voices["p2"]["vel"] = v1
            voices["p2"]["gate"] = 1
        else:
            voices["p2"]["gate"] = 0

        if len(global_note_stack) > 2:
            n2, v2 = global_note_stack[2]
            voices["tri"]["note"] = n2
            voices["tri"]["vel"] = v2
            voices["tri"]["gate"] = 1
        else:
            voices["tri"]["gate"] = 0

    bump_seq()
    mark_dirty()
    if VERBOSE_EVENTS:
        print(f"GLOBAL -> {global_note_stack}")


def arp_interval_s() -> float:
    return 0.03 + ((127 - arp_speed_cc) / 127.0) * 0.35


def arp_step() -> None:
    global arp_phase, arp_dir

    notes = len(global_note_stack)
    cycle_len = notes * (arp_octaves + 1)

    if not arp_enabled or notes <= 1:
        return

    if arp_mode == ARP_UP:
        arp_phase = (arp_phase + 1) % cycle_len
    elif arp_mode == ARP_DOWN:
        arp_phase = (arp_phase - 1) % cycle_len
    elif arp_mode == ARP_UPDOWN:
        if cycle_len <= 1:
            arp_phase = 0
        else:
            arp_phase += arp_dir
            if arp_phase >= cycle_len - 1:
                arp_phase = cycle_len - 1
                arp_dir = -1
            elif arp_phase <= 0:
                arp_phase = 0
                arp_dir = 1

    refresh_global_split()


def note_on(channel_1_based: int, note: int, velocity: int) -> None:
    voice_name = CHANNEL_TO_VOICE.get(channel_1_based)
    if voice_name:
        stack = [(n, v) for (n, v) in voice_note_stacks[voice_name] if n != clamp(note, 0, 127)]
        stack.insert(0, (note, velocity))
        set_voice_stack(voice_name, stack)
        return

    if channel_1_based == CHANNEL_GLOBAL:
        global global_note_stack
        global_note_stack = [(n, v) for (n, v) in global_note_stack if n != clamp(note, 0, 127)]
        global_note_stack.insert(0, (note, velocity))
        refresh_global_split()


def note_off(channel_1_based: int, note: int) -> None:
    voice_name = CHANNEL_TO_VOICE.get(channel_1_based)
    if voice_name:
        stack = [(n, v) for (n, v) in voice_note_stacks[voice_name] if n != clamp(note, 0, 127)]
        set_voice_stack(voice_name, stack)
        return

    if channel_1_based == CHANNEL_GLOBAL:
        if arp_latch:
            return
        global global_note_stack
        global_note_stack = [(n, v) for (n, v) in global_note_stack if n != clamp(note, 0, 127)]
        refresh_global_split()


def all_notes_off() -> None:
    global global_note_stack
    changed = False
    for voice in voices.values():
        if voice["gate"]:
            voice["gate"] = 0
            changed = True
    for key in voice_note_stacks:
        if voice_note_stacks[key]:
            voice_note_stacks[key] = []
            changed = True
    if global_note_stack:
        global_note_stack = []
        changed = True
    if changed:
        bump_seq()
        mark_dirty()
        if VERBOSE_EVENTS:
            print("ALL NOTES OFF")


def set_voice_control_value(voice_name: str, key: str, value: int) -> bool:
    if voice_name not in voice_params:
        return False

    if key == "A":
        voice_params[voice_name]["attack"] = clamp(value, 0, 127)
    elif key == "D":
        voice_params[voice_name]["decay"] = clamp(value, 0, 127)
    elif key == "S":
        voice_params[voice_name]["sustain"] = clamp(value, 0, 127)
    elif key == "R":
        voice_params[voice_name]["release"] = clamp(value, 0, 127)
    elif key == "VD":
        voice_params[voice_name]["vib_depth"] = clamp(value, 0, 127)
    elif key == "VR":
        voice_params[voice_name]["vib_rate"] = clamp(value, 0, 127)
    elif key == "GL":
        voice_params[voice_name]["glide"] = clamp(value, 0, 127)
    elif key == "NT":
        if voice_name != "noi":
            return False
        voice_params[voice_name]["timbre"] = clamp(value, 0, 127)
    elif key == "NM":
        if voice_name != "noi":
            return False
        voice_params[voice_name]["mode"] = 127 if value >= 64 else 0
    elif key == "DU":
        if voice_name not in ("p1", "p2"):
            return False
        voice_params[voice_name]["duty"] = clamp(value, 0, 3)
    else:
        return False

    return True


def set_control(key: str, value: int, voice_name: Optional[str] = None) -> None:
    key = key.upper().strip()
    value = int(value)

    if not voice_name or not set_voice_control_value(voice_name, key, value):
        return

    bump_seq()
    mark_dirty()
    if VERBOSE_EVENTS and voice_name:
        print(f"{voice_name.upper()} {key} -> {value}")
    elif VERBOSE_EVENTS:
        print(f"{key} -> {value}")


def broadcast_voice_control(key: str, value: int) -> None:
    changed = False
    for voice_name in ("p1", "p2", "tri", "noi"):
        changed = set_voice_control_value(voice_name, key, value) or changed

    if changed:
        bump_seq()
        mark_dirty()
        if VERBOSE_EVENTS:
            print(f"GLOBAL {key} -> {value}")


def parse_control_change(channel_1_based: int, cc: int, val: int) -> None:
    global arp_enabled, arp_mode, arp_speed_cc, arp_phase, arp_dir, arp_latch, arp_octaves

    if cc == 120 or cc == 123:
        all_notes_off()
        return

    if cc == 71:
        if channel_1_based == CHANNEL_GLOBAL:
            broadcast_voice_control("S", val)
        else:
            voice_name = CHANNEL_TO_VOICE.get(channel_1_based)
            if voice_name:
                set_control("S", val, voice_name)
            elif PRINT_IGNORED_CC:
                print(f"CC ignore CH{channel_1_based} cc={cc} val={val}")
    elif cc == 72:
        if channel_1_based == CHANNEL_GLOBAL:
            broadcast_voice_control("R", val)
        else:
            voice_name = CHANNEL_TO_VOICE.get(channel_1_based)
            if voice_name:
                set_control("R", val, voice_name)
            elif PRINT_IGNORED_CC:
                print(f"CC ignore CH{channel_1_based} cc={cc} val={val}")
    elif cc == 73:
        if channel_1_based == CHANNEL_GLOBAL:
            broadcast_voice_control("A", val)
        else:
            voice_name = CHANNEL_TO_VOICE.get(channel_1_based)
            if voice_name:
                set_control("A", val, voice_name)
            elif PRINT_IGNORED_CC:
                print(f"CC ignore CH{channel_1_based} cc={cc} val={val}")
    elif cc == 75:
        if channel_1_based == CHANNEL_GLOBAL:
            broadcast_voice_control("D", val)
        else:
            voice_name = CHANNEL_TO_VOICE.get(channel_1_based)
            if voice_name:
                set_control("D", val, voice_name)
            elif PRINT_IGNORED_CC:
                print(f"CC ignore CH{channel_1_based} cc={cc} val={val}")
    elif channel_1_based == CHANNEL_NOISE:
        if cc == 16:
            set_control("NT", val, "noi")
        elif cc == 1:
            set_control("NM", val, "noi")
        elif PRINT_IGNORED_CC:
            print(f"CC ignore CH{channel_1_based} cc={cc} val={val}")
    elif channel_1_based == CHANNEL_GLOBAL:
        if cc == 24:
            arp_enabled = val >= 64
            arp_phase = 0
            arp_dir = 1
            refresh_global_split()
        elif cc == 25:
            arp_latch = val >= 64
        elif cc == 26:
            if val < 43:
                arp_mode = ARP_UP
            elif val < 86:
                arp_mode = ARP_DOWN
            else:
                arp_mode = ARP_UPDOWN
            arp_phase = 0
            arp_dir = 1
            refresh_global_split()
        elif cc == 27:
            arp_speed_cc = clamp(val, 0, 127)
        elif cc == 28:
            if val < 43:
                arp_octaves = 0
            elif val < 86:
                arp_octaves = 1
            else:
                arp_octaves = 2
            arp_phase = 0
            arp_dir = 1
            refresh_global_split()
        elif cc == 1:
            broadcast_voice_control("VD", val)
        elif cc == 5:
            broadcast_voice_control("GL", val)
        elif cc == 16:
            broadcast_voice_control("DU", (val * 4) // 128)
        elif cc == 76:
            broadcast_voice_control("VR", val)
        elif PRINT_IGNORED_CC:
            print(f"CC ignore CH{channel_1_based} cc={cc} val={val}")
    else:
        voice_name = CHANNEL_TO_VOICE.get(channel_1_based)
        if cc == 1 and voice_name in voice_params:
            set_control("VD", val, voice_name)
        elif cc == 5 and voice_name in voice_params:
            set_control("GL", val, voice_name)
        elif cc == 16 and voice_name in ("p1", "p2"):
            set_control("DU", (val * 4) // 128, voice_name)
        elif cc == 76 and voice_name in voice_params:
            set_control("VR", val, voice_name)
        elif PRINT_IGNORED_CC:
            print(f"CC ignore CH{channel_1_based} cc={cc} val={val}")


def open_midi_input():
    if mido is None:
        print("mido n'est pas disponible.")
        return None

    try:
        names = mido.get_input_names()
    except Exception as e:
        print("Backend MIDI indisponible :", e)
        return None

    if not names:
        print("Aucune entree MIDI trouvee.")
        return None

    print("Entrees MIDI disponibles :")
    for name in names:
        print(" -", name)

    if MIDI_INPUT_NAME_HINT:
        for name in names:
            if MIDI_INPUT_NAME_HINT.lower() in name.lower():
                print("Ouverture MIDI :", name)
                return mido.open_input(name)
        print("Aucune entree MIDI ne correspond a :", MIDI_INPUT_NAME_HINT)
        return None

    print("Ouverture MIDI :", names[0])
    return mido.open_input(names[0])


def parse_midi_message(msg) -> None:
    channel_1_based = getattr(msg, "channel", -1) + 1

    if msg.type == "note_on":
        if msg.velocity == 0:
            note_off(channel_1_based, msg.note)
        else:
            note_on(channel_1_based, msg.note, msg.velocity)
        return

    if msg.type == "note_off":
        note_off(channel_1_based, msg.note)
        return

    if msg.type == "control_change":
        parse_control_change(channel_1_based, msg.control, msg.value)


def main() -> None:
    global last_arp_time
    midi_in = open_midi_input()
    mark_dirty()
    flush_state_if_due(force=True)
    last_arp_time = time.monotonic()

    print("Bridge separate-channels pret.")
    print(f"State file : {STATE_FILE}")
    print(f"MIDI hint : {MIDI_INPUT_NAME_HINT or 'NONE'}")
    print_state_summary()

    while True:
        if midi_in:
            for msg in midi_in.iter_pending():
                parse_midi_message(msg)

        if arp_enabled and len(global_note_stack) > 1:
            now = time.monotonic()
            if (now - last_arp_time) >= arp_interval_s():
                arp_step()
                last_arp_time = now

        flush_state_if_due()
        time.sleep(0.001)


if __name__ == "__main__":
    main()
