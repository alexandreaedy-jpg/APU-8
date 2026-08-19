# V2 Step 3 - READ_STATUS only

This is the first true protocol probe.

## Pico firmware

- unlocks safely
- decodes only one opcode:
  - raw `OUT = 0x01` (`READ_STATUS`)
- returns logical `D=10001`
  - `D4 = VALID = 1`
  - `D0 = PENDING = 1`

## NES ROM

- unlocks the Pico-side `245`
- continuously writes `READ_STATUS`
- continuously reads `$4017 & 0x1F`
- audio meaning:
  - `0x11` = expected good tone
  - `0x01` = `VALID` missing
  - `0x10` = `PENDING` missing
  - `0x00` = silence / no response
  - anything else = error tone
