# V18 D0 + CLK Redesign

## Why the current D0 path fails

- The current V18 path does **not** use the real controller-port clock wire.
- The Nano code explicitly documents that the green `CLK` wire is disconnected:
  - [NanoNesV18DN4.ino](C:/Users/mto1/Documents/NES_DEV/arduino/NanoNesV18DN4/NanoNesV18DN4.ino:20)
  - [NanoNesV18DN4.ino](C:/Users/mto1/Documents/NES_DEV/arduino/NanoNesV18DN4/NanoNesV18DN4.ino:27)
- The Nano runtime is driven from the latch/OUT pin interrupt, not from a true clock edge:
  - [NanoNesV18DN4.ino](C:/Users/mto1/Documents/NES_DEV/arduino/NanoNesV18DN4/NanoNesV18DN4.ino:384)
- The ROM currently abuses `$4016` writes as both latch and bit advance:
  - [main.c](C:/Users/mto1/Documents/NES_DEV/project-v18-DN4/main.c:504)
  - [main.c](C:/Users/mto1/Documents/NES_DEV/project-v18-DN4/main.c:511)
- This differs from the native NES controller protocol, where `$4016` is used to latch and successive reads from `$4016/$4017` generate the external shift clock.

## What NESdev / reference projects imply

- NESdev controller reading model:
  - write latch once
  - then read serial bits with the native read clock
- `ChipMaestro` does not use the controller-port serial protocol for musical timing. It pushes tiny register/data writes with a hardware handshake:
  - [ChipMaestro.ino](C:/Users/mto1/Documents/NES_DEV/_ref_ChipMaestro_thquinn/ChipMaestro.ino:623)
- `NESizer2` also avoids the controller-port transport bottleneck. It separates timing domains and applies modulation independently from note transport:
  - [task.c](C:/Users/mto1/Documents/NES_DEV/_ref_NESizer2/src/task/task.c:34)
  - [lfo.c](C:/Users/mto1/Documents/NES_DEV/_ref_NESizer2/src/lfo/lfo.c:29)
  - [modulation.c](C:/Users/mto1/Documents/NES_DEV/_ref_NESizer2/src/modulation/modulation.c:112)

## V18 redesign goals

1. Keep `D0` for urgent musical events only.
2. Move `D3` and `D4` completely out of the critical note-trigger path.
3. Use the **real NES CLK wire** as the bit-advance source.
4. Stop treating the controller port like a homemade packet bus with software-generated clock pulses.

## Proposed lane split

- `D0`: note/gate/trigger event lane only
- `D3`: ADSR + duty slow state lane
- `D4`: vibrato/LFO preset slow state lane

`D3` and `D4` must never be required for note-on/note-off timing.

## Proposed hardware change

- Connect the real NES controller `CLK` line to the Nano.
- Keep latch/OUT connected too, for frame start / sync.

Chosen Nano pin for the first implementation:

- `D13` (`PB5`, `PCINT5`) as `NES_CLK_PIN`

Reason:

- `D12` is already used for latch/OUT input.
- `D2`, `D3`, `D4` are already the three data lanes.
- `D5..D11` are already used by selectors.
- `D13` is the clean remaining digital pin with pin-change interrupt support.

Suggested behavior:

- latch edge: reset Nano shift position
- each real `CLK` edge: shift one bit from the prebuilt D0 event buffer
- no per-edge payload rebuild inside the ISR

## Proposed D0 event protocol

The key change is: **D0 becomes an event stream, not a full state snapshot.**

Minimal first-pass event frame:

- byte 0: header / event kind
- byte 1: sequence
- byte 2: voice + flags
- byte 3: note or payload

Suggested `byte 2` layout:

- bits 0-1: voice (`P1`, `P2`, `TRI`, `NOISE`)
- bit 2: gate
- bit 3: trigger
- bits 4-7: event subtype / reserved

Examples:

- `note on P1`: voice=`P1`, gate=1, trigger=1, note in byte 3
- `note off P1`: voice=`P1`, gate=0, trigger=0, note in byte 3
- `noise hit`: voice=`NOISE`, trigger=1, payload in byte 3

This gives one small transport unit per event, instead of resending all voices on every activity.

## ROM behavior

- D0 handler:
  - decode one event
  - update only the targeted voice state
  - do not touch unrelated channels
- D3 handler:
  - refresh only on explicit slow-lane polling or change detect
- D4 handler:
  - same idea, slow preset/config lane only

## Why this should be more precise

- no global 3-line re-read for every note event
- no note from `P2` forcing `P1` vibrato logic through the same heavy packet parse
- no need to keep `D3/D4` phase-aligned with urgent triggers
- closer to the natural controller-port clocking model documented by NESdev

## First implementation order

1. restore a known-readable ROM/Nano baseline
2. wire real `CLK`
3. add Nano ISR path based on `CLK` edges
4. replace D0 snapshot packet with one small event frame
5. keep D3/D4 unchanged at first
6. validate note precision before touching vibrato again
