# V2 Step 2 - Joypad D0..D4 cycle probe

This probe validates the **module -> NES** return path through the `74HCT245`.

## Pico side

Flash:
- [PicoNesV2A_Step2JoypadCycle.ino](/C:/Users/mto1/Documents/NES_DEV/arduino/PicoNesV2A_Step2JoypadCycle/PicoNesV2A_Step2JoypadCycle.ino)

Behavior:
- `74HCT245` starts disabled
- after `1 s`, it enables and drives:
  - `D0`
  - `D1`
  - `D2`
  - `D3`
  - `D4`
  - then `00000`
- each state is held for `2 s`

Serial log format:
- `D=abcde` means `D4 D3 D2 D1 D0`

## NES side

ROM:
- [V2_D0D4_CYCLE.nes](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/v2a_step2_joypad_cycle_probe/V2_D0D4_CYCLE.nes)

Behavior:
- reads `$4017 & 0x1F`
- maps one-hot values to 5 distinct pulse tones
- `00000` gives silence
- any invalid value (multiple bits high / unexpected) gives a fail tone

## Expected result

If the whole return path is correct, you should hear a slow repeating ladder:

1. tone for `00001`
2. tone for `00010`
3. tone for `00100`
4. tone for `01000`
5. tone for `10000`
6. silence

If the tones are missing, out of order, or replaced by the fail tone, the issue is on the `245 -> Joypad D0..D4 -> $4017` path.
