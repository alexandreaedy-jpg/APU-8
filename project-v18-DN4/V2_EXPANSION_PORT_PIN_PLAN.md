## V2 Expansion Port Pin Plan

Date: 2026-05-04

### Scope

Minimal set of NES expansion-port pins worth soldering first for a serious V2 proto.

This is based on the NES-001 bottom expansion-port pinout from NESdev.

Sources:

- https://www.nesdev.org/wiki/Expansion_port
- https://commons.wikimedia.org/wiki/File:Nintendo-NES-Mk1-Motherboard-Bottom.jpg

### Important orientation note

NESdev names the sides like this:

- **front** = controller ports / power / reset side of the console
- **back** = RF / AV side of the console

In the NESdev diagram, the connector is shown as:

```text
          (back)       NES       (front)
                    +-------\
               ...  |01   48| ...
               ...  |02   47| ...
                    ...
               ...  |24   25|
                    +-------/
```

So:

- left column is pins `01 -> 24`
- right column is pins `48 -> 25`

Before soldering the whole bundle, verify at least one or two pins with continuity against
their destination, because visual left/right can get confusing once the motherboard is flipped.

### Very important correction for V2 planning

The NES expansion port exposes:

- `CPU D0..D7`
- `A15`
- `/IRQ`
- `/NMI`
- `OUT0..OUT2`

But **it does not expose CPU `R/W`** on the bottom expansion port pinout documented by NESdev.

That means a first V2 should not assume a fully generic memory-mapped read/write peripheral.
The architecture will likely need to use:

- `CPU D0..D7`
- `A15`
- `OUT0..OUT2`
- `/IRQ`

as a more custom handshake/latch scheme.

### Minimal pins to bring out first

These are the pins I would definitely solder for V2 proto phase 1:

| Priority | Pin | Signal | Why |
|---|---:|---|---|
| must | 01 | `+5V` | logic power |
| must | 02 | `GND` | ground |
| must | 47 | `GND` | second ground, strongly recommended |
| must | 32 | `CPU D0` | data bus |
| must | 31 | `CPU D1` | data bus |
| must | 30 | `CPU D2` | data bus |
| must | 29 | `CPU D3` | data bus |
| must | 28 | `CPU D4` | data bus |
| must | 27 | `CPU D5` | data bus |
| must | 26 | `CPU D6` | data bus |
| must | 25 | `CPU D7` | data bus |
| must | 05 | `A15` | simplest address-side clue exposed on this port |
| must | 43 | `OUT0` | CPU-controlled output bit from `$4016`, useful as a control / strobe |
| should | 44 | `OUT1` | extra CPU-controlled output bit |
| should | 45 | `OUT2` | extra CPU-controlled output bit |
| should | 14 | `/IRQ` | very useful to signal the CPU, use **1 kΩ series resistor** |

### Useful optional pins

Only bring these out if you want spare options or debug flexibility:

| Priority | Pin | Signal | Note |
|---|---:|---|---|
| optional | 48 | `+5V` | second +5V point if mechanically convenient |
| optional | 04 | `/NMI` | open-collector, potentially useful later |
| optional | 11 | `/OE joypad 2` | only if you want controller-style timing experiments |
| optional | 37 | `/OE joypad 1` | same as above |
| optional | 12 | `joypad 1 /D1` | controller/debug only |
| optional | 20 | `joypad 2 /D1` | controller/debug only |
| optional | 03 | `Audio mix input` | only if later V2 returns audio into the NES |
| optional | 06..10, 38..42 | `EXP` pins | keep for later exploration, not phase 1 |

### Pins I would NOT prioritize right now

- `joypad` data lines
- `EXP0..EXP9`
- video/audio related lines

Those are interesting, but they are not the shortest path to a first serious V2 bus experiment.

### Suggested first nappe contents

If we want one sane first cable, I would take this set:

1. `02 GND`
2. `32 CPU D0`
3. `31 CPU D1`
4. `30 CPU D2`
5. `29 CPU D3`
6. `28 CPU D4`
7. `27 CPU D5`
8. `26 CPU D6`
9. `25 CPU D7`
10. `47 GND`
11. `05 A15`
12. `43 OUT0`
13. `44 OUT1`
14. `45 OUT2`
15. `14 /IRQ` through `1 kΩ`
16. `01 +5V`

That gives a compact and useful phase-1 bundle.

### Mechanical advice for soldering under the cart slot

- Keep the stripped length very short.
- Let each wire leave the pin almost flat, then bend away after a few millimeters.
- Do not build a vertical "hedgehog" of wires under the slot.
- Put strain relief farther away, not right on the pin row.
- Two grounds in the bundle are worth it.

### Bottom line

For a realistic first V2, the most valuable pins are:

- `CPU D0..D7`
- `A15`
- `OUT0..OUT2`
- `/IRQ`
- `+5V`
- `GND`

And the biggest architecture caveat is:

- **no CPU `R/W` line is exposed here**

So V2 should be designed around that fact from day one.
