import os
import warnings
import wave

warnings.filterwarnings("ignore", category=DeprecationWarning)
import aifc  # type: ignore


STEP_DIR = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_ROOT = os.path.dirname(os.path.dirname(STEP_DIR))
MILESTONE_ROOT = os.path.join(
    WORKSPACE_ROOT,
    "milestones",
    "APU8_failed_fastdiag_20260423-142117",
    "project-separate-channels",
)
KIT_DIR = os.path.join(MILESTONE_ROOT, "dmc-selected", "playable-kit", "dmc")
SRC_DIR = os.path.join(MILESTONE_ROOT, "dmc-source-samples")
OUT_ASM = os.path.join(STEP_DIR, "dmc_samples.s")


SPECS = [
    {
        "source_kind": "dmc",
        "label": "dmc_kick",
        "filename": "02_kick_smb3.dmc",
        "role": "kick",
        "rate_index": 0x0C,
        "start_reg": 0xC0,
        "length_reg": 0x34,
        "gain": 1.9,
        "pitch_semitones": 0.5,
    },
    {
        "source_kind": "dmc",
        "label": "dmc_snare",
        "filename": "05_snare_smb3_b.dmc",
        "role": "snare",
        "rate_index": 0x0C,
        "start_reg": 0xCE,
        "length_reg": 0x1C,
        "gain": 1.7,
        "pitch_semitones": 0.5,
    },
    {
        "source_kind": "dmc",
        "label": "dmc_rim",
        "filename": "06_rimshot_smb3.dmc",
        "role": "rimshot",
        "rate_index": 0x0C,
        "start_reg": 0xD6,
        "length_reg": 0x0F,
        "gain": 1.15,
        "pitch_semitones": 0.5,
    },
    {
        "source_kind": "audio",
        "label": "dmc_clap",
        "filename": "808_clap.aif",
        "role": "clap",
        "rate_index": 0x0C,
        "start_reg": 0xDA,
        "length_reg": 0x18,
        "source_ms": 52.0,
        "mode": "edge_binary",
    },
    {
        "source_kind": "audio",
        "label": "dmc_hat",
        "filename": "808_hihat_closed.aif",
        "role": "hat_closed",
        "rate_index": 0x0C,
        "start_reg": 0xE1,
        "length_reg": 0x10,
        "source_ms": 24.0,
        "mode": "edge_binary",
    },
]


def dmc_length_bytes(length_reg: int) -> int:
    return length_reg * 16 + 1


def decode_dmc_bytes(data: bytes, initial_level: int = 64) -> list[float]:
    level = max(0, min(127, initial_level))
    samples: list[float] = []
    for byte_value in data:
        for bit_index in range(8):
            bit = (byte_value >> bit_index) & 1
            if bit:
                if level <= 125:
                    level += 2
            else:
                if level >= 2:
                    level -= 2
            samples.append((level - 64) / 63.0)
    return samples


def encode_dmc(samples: list[float], start_level: int = 64) -> bytes:
    desired: list[int] = []
    for sample in samples:
        level = int(round(64 + sample * 62))
        if level < 0:
            level = 0
        if level > 127:
            level = 127
        desired.append(level)

    current = max(0, min(127, start_level))
    out = bytearray()
    for offset in range(0, len(desired), 8):
        packed = 0
        for bit_index in range(8):
            target = desired[offset + bit_index]
            up = current + 2 if current <= 125 else 127
            down = current - 2 if current >= 2 else 0
            if abs(target - up) <= abs(target - down):
                current = up
                packed |= 1 << bit_index
            else:
                current = down
        out.append(packed)
    return bytes(out)


def resample_linear(samples: list[float], target_count: int) -> list[float]:
    if not samples:
        return [0.0] * target_count
    if len(samples) == 1:
        return [samples[0]] * target_count

    result: list[float] = []
    last_index = len(samples) - 1
    for i in range(target_count):
        pos = (i * last_index) / max(1, target_count - 1)
        left = int(pos)
        right = min(left + 1, last_index)
        frac = pos - left
        result.append(samples[left] * (1.0 - frac) + samples[right] * frac)
    return result


def simulate_levels(data: bytes, start_level: int) -> list[int]:
    level = max(0, min(127, start_level & 0x7F))
    levels: list[int] = []
    for byte_value in data:
        for bit_index in range(8):
            bit = (byte_value >> bit_index) & 1
            if bit:
                if level <= 125:
                    level += 2
            else:
                if level >= 2:
                    level -= 2
            levels.append(level)
    return levels


def choose_start_level(data: bytes) -> int:
    best_level = 64
    best_score = None
    for start_level in range(2, 127, 2):
        levels = simulate_levels(data, start_level)
        lo = min(levels)
        hi = max(levels)
        peak_to_peak = hi - lo
        clipped = sum(1 for value in levels if value <= 0 or value >= 127)
        near_rails = sum(1 for value in levels if value <= 2 or value >= 125)
        center_bias = abs(((lo + hi) // 2) - 64)
        score = clipped * 100000 + near_rails * 200 + center_bias * 8 - peak_to_peak * 64
        if best_score is None or score < best_score:
            best_score = score
            best_level = start_level
    return best_level


def apply_gain(data: bytes, gain: float, start_level: int = 64) -> bytes:
    if abs(gain - 1.0) < 0.001:
        return data
    shaped: list[float] = []
    for sample in decode_dmc_bytes(data, start_level):
        value = max(-1.0, min(1.0, sample * gain))
        shaped.append(value)
    return encode_dmc(shaped, start_level)


def apply_pitch(data: bytes, semitones: float, start_level: int = 64) -> bytes:
    if abs(semitones) < 0.001:
        return data
    decoded = decode_dmc_bytes(data, start_level)
    target_count = len(decoded)
    ratio = 2.0 ** (semitones / 12.0)
    source_count = int(round(target_count * ratio))
    if source_count < 8:
        source_count = 8
    if source_count > len(decoded):
        source_count = len(decoded)
    shifted = resample_linear(decoded[:source_count], target_count)
    return encode_dmc(shifted, start_level)


def read_audio_file(path: str):
    ext = os.path.splitext(path)[1].lower()
    if ext in (".aif", ".aiff"):
        with aifc.open(path, "rb") as f:
            return (
                f.readframes(f.getnframes()),
                f.getnchannels(),
                f.getsampwidth(),
                f.getframerate(),
                True,
            )
    with wave.open(path, "rb") as f:
        return (
            f.readframes(f.getnframes()),
            f.getnchannels(),
            f.getsampwidth(),
            f.getframerate(),
            False,
        )


def decode_mono_samples(raw: bytes, channels: int, sample_width: int, big_endian: bool) -> list[float]:
    frame_size = channels * sample_width
    bits = sample_width * 8
    sign_bit = 1 << (bits - 1)
    full_scale = float(sign_bit)
    byteorder = "big" if big_endian else "little"
    samples: list[float] = []
    total_frames = len(raw) // frame_size
    for frame_index in range(total_frames):
        base = frame_index * frame_size
        acc = 0.0
        for channel_index in range(channels):
            start = base + channel_index * sample_width
            chunk = raw[start:start + sample_width]
            if sample_width == 1:
                value = chunk[0] - 128
            else:
                value = int.from_bytes(chunk, byteorder=byteorder, signed=False)
                if value & sign_bit:
                    value -= 1 << bits
            acc += value / full_scale
        samples.append(acc / channels)
    return samples


def crop_audio(samples: list[float], frame_rate: int, source_ms: float, mode: str) -> list[float]:
    count = max(1, int(round(frame_rate * source_ms / 1000.0)))
    count = min(count, len(samples))
    cropped = samples[:count] or [0.0]
    mean = sum(cropped) / len(cropped)
    cropped = [sample - mean for sample in cropped]
    peak = max(abs(sample) for sample in cropped) or 1.0
    gain = 0.98 / peak
    cropped = [max(-1.0, min(1.0, sample * gain)) for sample in cropped]
    if mode == "edge_binary":
        shaped: list[float] = []
        prev = 0.0
        for sample in cropped:
            edge = sample - (prev * 0.88)
            prev = sample
            shaped.append(edge)
        peak = max(abs(sample) for sample in shaped) or 1.0
        return [1.0 if (sample / peak) >= 0.0 else -1.0 for sample in shaped]
    return cropped


def format_bytes(byte_values: bytes) -> str:
    lines = []
    for i in range(0, len(byte_values), 8):
        chunk = byte_values[i:i + 8]
        lines.append("    .byte " + ", ".join(f"${value:02X}" for value in chunk))
    return "\n".join(lines)


def build_dmc_sample(spec: dict) -> dict:
    path = os.path.join(KIT_DIR, spec["filename"])
    with open(path, "rb") as f:
        source = f.read()
    target_len = dmc_length_bytes(spec["length_reg"])
    if len(source) < target_len:
        raise RuntimeError(f"{spec['filename']} too short: {len(source)} < {target_len}")
    byte_values = source[:target_len]
    byte_values = apply_gain(byte_values, spec.get("gain", 1.0))
    byte_values = apply_pitch(byte_values, spec.get("pitch_semitones", 0.0))
    level = choose_start_level(byte_values)
    return {**spec, "level": level, "bytes": byte_values}


def build_audio_sample(spec: dict) -> dict:
    path = os.path.join(SRC_DIR, spec["filename"])
    raw, channels, sample_width, frame_rate, big_endian = read_audio_file(path)
    mono = decode_mono_samples(raw, channels, sample_width, big_endian)
    cropped = crop_audio(mono, frame_rate, spec["source_ms"], spec.get("mode", "level"))
    target_count = dmc_length_bytes(spec["length_reg"]) * 8
    resampled = resample_linear(cropped, target_count)
    level = 64
    byte_values = encode_dmc(resampled, level)
    level = choose_start_level(byte_values)
    byte_values = encode_dmc(resampled, level)
    return {**spec, "level": level, "bytes": byte_values}


def build_sample(spec: dict) -> dict:
    if spec["source_kind"] == "dmc":
        return build_dmc_sample(spec)
    return build_audio_sample(spec)


def main() -> None:
    generated = [build_sample(spec) for spec in SPECS]
    lines = [
        "; Auto-generated by step4 make_dmc_bank.py",
        "; Hybrid bank: kick / snare / rim from playable-kit, clap / hat from local 808 sources",
        "; Segment starts at $F000 in nrom_256_vert.cfg.",
        "; Sample slots: $F000/$F380/$F580/$F680/$F840 -> start regs $C0/$CE/$D6/$DA/$E1",
        "",
        '.segment "SAMPLES"',
        "",
    ]

    for sample in generated:
        lines.append(".align 64")
        lines.append(f"{sample['label']}:")
        lines.append(
            f"; role={sample['role']} source={sample['filename']} "
            f"rate=${sample['rate_index']:02X} len=${sample['length_reg']:02X} "
            f"start=${sample['start_reg']:02X} level=${sample['level']:02X}"
        )
        lines.append(format_bytes(sample["bytes"]))
        lines.append("")

    with open(OUT_ASM, "w", encoding="ascii", newline="\n") as f:
        f.write("\n".join(lines).rstrip() + "\n")

    print(f"Generated {OUT_ASM}")
    for sample in generated:
        print(
            f"{sample['label']}: {sample['filename']} "
            f"(rate=${sample['rate_index']:02X}, start=${sample['start_reg']:02X}, "
            f"len=${sample['length_reg']:02X}, level=${sample['level']:02X})"
        )


if __name__ == "__main__":
    main()
