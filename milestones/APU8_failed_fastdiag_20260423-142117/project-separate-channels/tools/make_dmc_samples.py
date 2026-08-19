import os
import warnings
import wave

warnings.filterwarnings("ignore", category=DeprecationWarning)
import aifc  # type: ignore


PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC_DIR = os.path.join(PROJECT_DIR, "dmc-source-samples")
OUT_ASM = os.path.join(PROJECT_DIR, "dmc_samples.s")


SPECS = [
    {
        "label": "dmc_kick",
        "filename": "808_kick.aif",
        "source_ms": 58.0,
        "mode": "level",
        "rate_index": 0x00,
        "start_reg": 0xFC,
        "length_reg": 0x03,
    },
    {
        "label": "dmc_snare",
        "filename": "808_snare.aif",
        "source_ms": 52.0,
        "mode": "edge_binary",
        "rate_index": 0x03,
        "start_reg": 0xFD,
        "length_reg": 0x03,
    },
    {
        "label": "dmc_hat",
        "filename": "808_hihat_closed.aif",
        "source_ms": 20.0,
        "mode": "edge_binary",
        "rate_index": 0x09,
        "start_reg": 0xFE,
        "length_reg": 0x03,
    },
]


def read_audio_file(path):
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


def decode_mono_samples(raw, channels, sample_width, frame_rate, big_endian):
    if channels <= 0:
        raise ValueError("invalid channel count")

    frame_size = channels * sample_width
    if frame_size <= 0:
        raise ValueError("invalid frame size")

    total_frames = len(raw) // frame_size
    bits = sample_width * 8
    sign_bit = 1 << (bits - 1)
    full_scale = float(sign_bit)
    byteorder = "big" if big_endian else "little"
    samples = []

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

    return samples, frame_rate


def crop_samples(samples, frame_rate, source_ms, mode):
    count = max(1, int(round(frame_rate * source_ms / 1000.0)))
    count = min(count, len(samples))
    cropped = samples[:count]
    if not cropped:
        return [0.0]
    mean = sum(cropped) / len(cropped)
    cropped = [sample - mean for sample in cropped]
    peak = max(abs(sample) for sample in cropped) or 1.0
    gain = 0.98 / peak
    cropped = [max(-1.0, min(1.0, sample * gain)) for sample in cropped]

    if mode == "edge_binary":
        shaped = []
        prev = 0.0
        for sample in cropped:
            edge = sample - (prev * 0.88)
            prev = sample
            shaped.append(edge)
        peak = max(abs(sample) for sample in shaped) or 1.0
        return [1.0 if (sample / peak) >= 0.0 else -1.0 for sample in shaped]

    shaped = []
    for sample in cropped:
        sample *= 1.45
        if sample > 1.0:
            sample = 1.0
        elif sample < -1.0:
            sample = -1.0
        shaped.append(sample)
    return shaped


def resample_linear(samples, target_count):
    if len(samples) == 1:
        return [samples[0]] * target_count
    result = []
    last_index = len(samples) - 1
    for i in range(target_count):
        pos = (i * last_index) / max(1, target_count - 1)
        left = int(pos)
        right = min(left + 1, last_index)
        frac = pos - left
        value = samples[left] * (1.0 - frac) + samples[right] * frac
        result.append(value)
    return result


def dmc_length_bytes(length_reg):
    return length_reg * 16 + 1


def encode_dmc(samples, start_level=64):
    desired = []
    for sample in samples:
        level = int(round(64 + sample * 62))
        if level < 0:
            level = 0
        if level > 127:
            level = 127
        desired.append(level)

    current = max(0, min(127, start_level))
    bytes_out = []
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
        bytes_out.append(packed)
    return bytes_out


def format_bytes(byte_values):
    lines = []
    for i in range(0, len(byte_values), 8):
        chunk = byte_values[i:i + 8]
        lines.append("    .byte " + ", ".join(f"${value:02X}" for value in chunk))
    return "\n".join(lines)


def build_sample(spec):
    path = os.path.join(SRC_DIR, spec["filename"])
    raw, channels, sample_width, frame_rate, big_endian = read_audio_file(path)
    mono, frame_rate = decode_mono_samples(raw, channels, sample_width, frame_rate, big_endian)
    cropped = crop_samples(mono, frame_rate, spec["source_ms"], spec.get("mode", "level"))
    target_count = dmc_length_bytes(spec["length_reg"]) * 8
    resampled = resample_linear(cropped, target_count)
    byte_values = encode_dmc(resampled)
    return {
        "label": spec["label"],
        "filename": spec["filename"],
        "rate_index": spec["rate_index"],
        "start_reg": spec["start_reg"],
        "length_reg": spec["length_reg"],
        "source_ms": spec["source_ms"],
        "bytes": byte_values,
    }


def main():
    generated = [build_sample(spec) for spec in SPECS]

    lines = [
        "; Auto-generated from dmc-source-samples by tools/make_dmc_samples.py",
        "; Segment starts at $FF00 in nrom_256_vert.cfg.",
        '; Sample slots: $FF00/$FF40/$FF80 -> start regs $FC/$FD/$FE',
        "",
        '.segment "SAMPLES"',
        "",
    ]

    for sample in generated:
        lines.append(".align 64")
        lines.append(f"{sample['label']}:")
        lines.append(
            f"; source={sample['filename']} crop={sample['source_ms']}ms rate=${sample['rate_index']:02X} len=${sample['length_reg']:02X} start=${sample['start_reg']:02X}"
        )
        lines.append(format_bytes(sample["bytes"]))
        lines.append("")

    with open(OUT_ASM, "w", encoding="ascii", newline="\n") as f:
        f.write("\n".join(lines).rstrip() + "\n")

    print(f"Generated {OUT_ASM}")
    for sample in generated:
        print(
            f"{sample['label']}: {len(sample['bytes'])} bytes from {sample['filename']} "
            f"(rate=${sample['rate_index']:02X}, start=${sample['start_reg']:02X}, len=${sample['length_reg']:02X})"
        )


if __name__ == "__main__":
    main()
