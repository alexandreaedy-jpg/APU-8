# V18.1 Proto Wiring (First Bring-up)

## Goal

Bring up a **D0-only** controller-style transport using:

- `1x 74HC595`
- `1x CD4021`
- the existing Nano
- the existing NES `LATCH` and `CLK` wires

This is a **first hardware validation**, not the final all-features wiring.

For this first test:

- `D0` becomes hardware-shifted
- `D3` and `D4` are temporarily ignored in the critical path
- `P1/P2/TRI` only
- `NOISE` stays out

## Why 74HC595 + 4021

The Nano does **not** have 8 spare output pins for directly loading a `4021`.

So the practical first proto is:

- Nano serial-loads a byte into `74HC595`
- `74HC595` parallel outputs feed the `4021` inputs
- NES `LATCH` loads the `4021`
- NES `CLK` shifts the byte out on controller-port `D0`

This lets the Nano keep its existing control inputs while we validate the urgent event lane.

## Power

Use only **one 5V source**:

- `Nano 5V` -> breadboard logic power
- `Nano GND` -> breadboard ground
- `NES GND` -> same common ground

Do **not** tie the controller-port `+5V` to the Nano `5V` for this first proto.

## Decoupling

Put:

- `100 nF` ceramic between `VCC` and `GND` on the `74HC595`
- `100 nF` ceramic between `VCC` and `GND` on the `4021`

Place each capacitor as close as possible to the chip power pins.

## Keep / disconnect for first test

Keep:

- NES `GND` -> Nano / breadboard GND

Change:

- remove the direct Nano `D2 -> NES D0` link
- NES `D0` will now come from the `4021` serial output

Optional for isolation:

- leave existing `D3` and `D4` hardware disconnected from the test path for now

Optional only:

- NES `OUT/LATCH` -> Nano `D12` as a monitor input
- NES `CLK` -> Nano `D13` as a monitor input

For the **current D0-only proto sketch**, the Nano does not actively use
`D12/D13` to drive the transfer. The `4021` is what actually consumes
`LATCH/CLK`. So `D12/D13` on the Nano are harmless if connected as inputs,
but they are **not required** for this first bring-up.

## Nano pins used for the 74HC595

For the first proto, dedicate:

- `Nano D2` -> `74HC595 DS`
- `Nano D3` -> `74HC595 SHCP`
- `Nano D4` -> `74HC595 STCP`

`Nano D1` is intentionally avoided here.

This means:

- `D0` is no longer driven directly by the Nano
- `D3` slow-lane direct output is temporarily sacrificed during the bring-up
- `D4` slow-lane direct output can remain unused for now

That is intentional. First validate `D0`.

## 74HC595 wiring

### 74HC595 power/control

- pin `16` (`VCC`) -> `+5V`
- pin `8` (`GND`) -> `GND`
- pin `10` (`MR`, active low) -> `+5V`
- pin `13` (`OE`, active low) -> `GND`
- pin `14` (`DS`) -> `Nano D2`
- pin `11` (`SHCP`) -> `Nano D3`
- pin `12` (`STCP`) -> `Nano D4`
- pin `9` (`Q7S`) -> not connected for now

### 74HC595 outputs to 4021 inputs

Because the NES reads the `4021` starting from **Q8 first**, and our ROM byte decoder builds bytes **LSB-first**, we map bit 0 to `PI-8`, bit 1 to `PI-7`, etc.

Connect:

- `74HC595 Q0` pin `15` -> `4021 PI-8` pin `1`
- `74HC595 Q1` pin `1`  -> `4021 PI-7` pin `15`
- `74HC595 Q2` pin `2`  -> `4021 PI-6` pin `14`
- `74HC595 Q3` pin `3`  -> `4021 PI-5` pin `13`
- `74HC595 Q4` pin `4`  -> `4021 PI-4` pin `4`
- `74HC595 Q5` pin `5`  -> `4021 PI-3` pin `5`
- `74HC595 Q6` pin `6`  -> `4021 PI-2` pin `6`
- `74HC595 Q7` pin `7`  -> `4021 PI-1` pin `7`

## 4021 wiring

### 4021 power/basic pins

- pin `16` (`VCC`) -> `+5V`
- pin `8` (`VSS`) -> `GND`
- pin `11` (`Serial In`) -> `GND`

### NES controller-facing pins

- pin `9` (`Parallel/Serial Control`) -> NES `OUT/LATCH`
- pin `10` (`Clock`) -> NES `CLK`
- pin `3` (`Q8`) -> NES controller-port `D0`

Unused for now:

- pin `2` (`Q6`) -> not connected
- pin `12` (`Q7`) -> not connected

## Signal summary

### Nano -> logic

- `D2` -> `74HC595 DS`
- `D3` -> `74HC595 SHCP`
- `D4` -> `74HC595 STCP`

### NES -> logic

- NES `OUT/LATCH` -> `4021 pin 9`
- NES `CLK` -> `4021 pin 10`

Optional monitor only:

- NES `OUT/LATCH` -> Nano `D12`
- NES `CLK` -> Nano `D13`

### Logic -> NES

- `4021 pin 3 (Q8)` -> NES `D0`

## First test strategy

### Hardware-only sanity

Before booting a ROM:

- confirm `+5V` and `GND` on both ICs
- confirm no direct Nano `D2 -> NES D0` remains
- confirm `4021 Q8` is the only source to NES `D0`

### Software bring-up target

For the first software pass:

- ROM fast path reads only `D0`
- ignore `D3`
- ignore `D4`
- decode only the compact `P1/P2/TRI` 8-byte frame

### Success condition

We are not yet judging musical quality.

First success is simply:

- notes come back
- repeated note reads are stable
- `P1/P2/TRI` stop cross-triggering wildly

If that works, then we can reintroduce `D3`, then `D4`.

## Important warning

Do **not** try to bring back `D3` and `D4` in the same breadboard pass that validates `D0`.

This proto is successful if it answers one question only:

**Does hardware-shifted `D0` restore a stable urgent trigger lane?**
