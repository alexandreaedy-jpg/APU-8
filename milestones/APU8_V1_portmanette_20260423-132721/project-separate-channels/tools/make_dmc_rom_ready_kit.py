import os
PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KIT_DIR = os.path.join(PROJECT_DIR, "dmc-selected", "playable-kit", "dmc")
OUT_ASM = os.path.join(PROJECT_DIR, "dmc_samples.s")


SPECS = [
    {
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
        "label": "dmc_voice",
        "filename": "12_voice_game_over.dmc",
        "role": "voice_fx",
        "rate_index": 0x0C,
        "start_reg": 0xDA,
        "length_reg": 0x94,
        "gain": 1.0,
        "pitch_semitones": 0.0,
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
                packed |= (1 << bit_index)
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
        score = (
            clipped * 100000
            + near_rails * 200
            + center_bias * 8
            - peak_to_peak * 64
        )

        if best_score is None or score < best_score:
            best_score = score
            best_level = start_level

    return best_level


def apply_gain(data: bytes, gain: float, start_level: int = 64) -> bytes:
    if abs(gain - 1.0) < 0.001:
        return data

    shaped: list[float] = []
    for sample in decode_dmc_bytes(data, start_level):
        value = sample * gain
        if value > 1.0:
            value = 1.0
        elif value < -1.0:
            value = -1.0
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


def format_bytes(byte_values: bytes) -> str:
    lines = []
    for i in range(0, len(byte_values), 8):
        chunk = byte_values[i:i + 8]
        lines.append("    .byte " + ", ".join(f"${value:02X}" for value in chunk))
    return "\n".join(lines)


def build_sample(spec: dict) -> dict:
    path = os.path.join(KIT_DIR, spec["filename"])
    with open(path, "rb") as f:
        source = f.read()

    target_len = dmc_length_bytes(spec["length_reg"])
    if len(source) < target_len:
        raise RuntimeError(
            f"{spec['filename']} too short for len=${spec['length_reg']:02X} "
            f"({len(source)} bytes < {target_len})"
        )

    byte_values = source[:target_len]
    byte_values = apply_gain(byte_values, spec.get("gain", 1.0))
    byte_values = apply_pitch(byte_values, spec.get("pitch_semitones", 0.0))

    return {
        **spec,
        "byte_count": target_len,
        "level": choose_start_level(byte_values),
        "bytes": byte_values,
    }


def main() -> None:
    generated = [build_sample(spec) for spec in SPECS]

    lines = [
        "; Auto-generated from dmc-selected/playable-kit by tools/make_dmc_rom_ready_kit.py",
        "; ROM-ready compact bank: kick / snare / rim / voice",
        "; Segment starts at $F000 in nrom_256_vert.cfg.",
        "; Sample slots: $F000/$F380/$F580/$F680 -> start regs $C0/$CE/$D6/$DA",
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
            f"{sample['label']}: {sample['filename']} -> {sample['byte_count']} bytes "
            f"(rate=${sample['rate_index']:02X}, start=${sample['start_reg']:02X}, len=${sample['length_reg']:02X})"
        )


if __name__ == "__main__":
    main()
