import argparse
import os
import wave


PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_INPUT_DIR = os.path.join(PROJECT_DIR, "dmc-source-samples")
DEFAULT_OUTPUT_DIR = os.path.join(PROJECT_DIR, "dmc-preview-wav")

# Approximate NTSC DMC playback sample rates by $4010 rate index.
DMC_SAMPLE_RATES = [
    4181,
    4709,
    5265,
    5593,
    6258,
    7046,
    7919,
    8363,
    9419,
    11186,
    12604,
    13983,
    16884,
    21307,
    24858,
    33144,
]


def decode_dmc_bytes(data: bytes, initial_level: int) -> list[int]:
    level = max(0, min(127, initial_level))
    samples: list[int] = []

    for byte_value in data:
        for bit_index in range(8):
            bit = (byte_value >> bit_index) & 1
            if bit:
                if level <= 125:
                    level += 2
            else:
                if level >= 2:
                    level -= 2
            centered = level - 64
            pcm = int((centered / 63.0) * 32767.0)
            if pcm > 32767:
                pcm = 32767
            elif pcm < -32768:
                pcm = -32768
            samples.append(pcm)

    return samples


def write_wav(path: str, samples: list[int], sample_rate: int) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, "wb") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)
        frames = bytearray()
        for sample in samples:
            frames.extend(int(sample).to_bytes(2, byteorder="little", signed=True))
        wav_file.writeframes(bytes(frames))


def convert_tree(input_dir: str, output_dir: str, rate_index: int, initial_level: int) -> int:
    sample_rate = DMC_SAMPLE_RATES[rate_index]
    converted = 0

    for root, _, files in os.walk(input_dir):
        for name in files:
            if not name.lower().endswith(".dmc"):
                continue

            source_path = os.path.join(root, name)
            rel_path = os.path.relpath(source_path, input_dir)
            rel_base, _ = os.path.splitext(rel_path)
            dest_path = os.path.join(
                output_dir,
                f"rate_{rate_index:02X}_{sample_rate}hz",
                rel_base + f"_r{rate_index:02X}.wav",
            )

            with open(source_path, "rb") as f:
                data = f.read()

            samples = decode_dmc_bytes(data, initial_level)
            write_wav(dest_path, samples, sample_rate)
            converted += 1

    return converted


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert .dmc files to preview .wav files.")
    parser.add_argument("--input-dir", default=DEFAULT_INPUT_DIR, help="Directory containing .dmc files.")
    parser.add_argument("--output-dir", default=DEFAULT_OUTPUT_DIR, help="Directory to write .wav previews.")
    parser.add_argument(
        "--rate-index",
        type=lambda s: int(s, 0),
        default=0x0C,
        help="NES DMC rate index (0x00..0x0F). Default: 0x0C.",
    )
    parser.add_argument(
        "--initial-level",
        type=int,
        default=64,
        help="Initial DAC level used for decode preview. Default: 64.",
    )
    args = parser.parse_args()

    if args.rate_index < 0 or args.rate_index >= len(DMC_SAMPLE_RATES):
        raise SystemExit("rate-index must be between 0x00 and 0x0F")

    converted = convert_tree(
        os.path.abspath(args.input_dir),
        os.path.abspath(args.output_dir),
        args.rate_index,
        args.initial_level,
    )

    print(
        f"Converted {converted} DMC files to WAV in "
        f"{os.path.join(os.path.abspath(args.output_dir), f'rate_{args.rate_index:02X}_{DMC_SAMPLE_RATES[args.rate_index]}hz')}"
    )


if __name__ == "__main__":
    main()
