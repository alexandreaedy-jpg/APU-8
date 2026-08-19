# V2 Proto 1 Wiring Freeze

Date: 2026-05-08 23:26:03

## Scope

This milestone freezes the **V2 proto 1 wiring and breadboard architecture** after the controller-port `D3/D4` work was paused and the project pivoted to the NES expansion port.

It captures:
- the retained `16-wire` ribbon order
- the **safe proto 1** signal bundle
- the physical split between the two breadboards
- the current "already soldered" vs "remaining to wire" state

## Frozen reference files

- [V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04.md](/C:/Users/mto1/Documents/NES_DEV/milestones/V2_PROTO1_WIRING_FREEZE_20260508-232603/V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04.md)
- [V2_EXPANSION_PORT_PIN_PLAN.md](/C:/Users/mto1/Documents/NES_DEV/milestones/V2_PROTO1_WIRING_FREEZE_20260508-232603/V2_EXPANSION_PORT_PIN_PLAN.md)
- [V2_PROTO1_BREADBOARD_ARCHITECTURE.md](/C:/Users/mto1/Documents/NES_DEV/milestones/V2_PROTO1_WIRING_FREEZE_20260508-232603/V2_PROTO1_BREADBOARD_ARCHITECTURE.md)
- [V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04_rev4.pdf](/C:/Users/mto1/Documents/NES_DEV/milestones/V2_PROTO1_WIRING_FREEZE_20260508-232603/V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04_rev4.pdf)

## Checksums

- `V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04.md`
  `7F60732179FE48766D3404F1AEEE2EA54429B7A1F8E65FB6BEAD790B65AA8A3A`
- `V2_EXPANSION_PORT_PIN_PLAN.md`
  `5EF3E450BEBEB3424B68D4FFF53AC3C485922474892795D476010C6A9DC01CA5`
- `V2_PROTO1_BREADBOARD_ARCHITECTURE.md`
  `59ACC54CDD47113F480FA21F572CE66F47164B9889483712B0551A9885F845DB`
- `V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04_rev4.pdf`
  `A5349D224A6DA070734DD91409C9863F83A2531D661FB2ABAD6C8BDDAD61A658`

## Frozen proto 1 decisions

- **Small breadboard**:
  - `RP2040 Pico`
  - MIDI optocoupler
  - Pico powered by `USB`
  - opto powered by `NES +5V`
- **Large breadboard**:
  - all NES communication logic
  - `2 x 74HC4050`
  - `1 x 74HCT245`
  - expansion-port ribbon landing area

## Safe proto 1 ribbon content

Ordered ribbon mapping:

1. `02` = `GND2`
2. `30` = `CPU D2`
3. `31` = `CPU D1`
4. `32` = `CPU D0`
5. `11` = `/OE2`
6. `05` = `A15`
7. `18` = `Joypad D4`
8. `16` = `Joypad D3`
9. `15` = `Joypad D2`
10. `20` = `Joypad D1`
11. `19` = `Joypad D0`
12. `43` = `OUT0`
13. `44` = `OUT1`
14. `45` = `OUT2`
15. `47` = `GND`
16. `48` = `+5V`

## State at freeze time

Already soldered on NES side:
- `48 +5V`
- `47 GND`
- `45 OUT2`
- `44 OUT1`
- `43 OUT0`
- `19 Joypad D0`
- `20 Joypad D1`
- `15 Joypad D2`
- `16 Joypad D3`
- `18 Joypad D4`
- `02 GND2`
- `11 /OE2`
- `05 A15`
- `32 CPU D0`
- `31 CPU D1`
- `30 CPU D2`

## Explicitly out of scope for proto 1

- `/IRQ`
- `CPU D3..D7`
- `R/W`
- `M2`
- any controller-port `D3/D4` comeback

## Next step after the pause

Resume with:
1. physical placement on the **large breadboard**
2. `74HC4050` / `74HCT245` power rails and decoupling
3. `NES -> Pico` bring-up first
4. only then activate `245` output path toward the NES
