# V2 Step 4 - Atomic queue probe

This probe validates the first full packet transaction with `ACK_AND_NEXT`.

## Pico firmware

- unlocks safely
- exposes a fixed 2-event queue
- keeps the current event frozen until `ACK_AND_NEXT`
- serves:
  - `READ_STATUS`
  - `READ_REG_ID`
  - `READ_VALUE_LO`
  - `READ_VALUE_HI`
  - `ACK_AND_NEXT`

Queue contents:

- event 0: `reg_id=0x0`, `value=0x3C`
- event 1: `reg_id=0x5`, `value=0xA7`

## NES ROM

- unlocks the Pico-side `245`
- reads a full packet
- reads the same fields twice before `ACK` to prove atomicity
- `ACK`s to advance to the next event
- checks that the queue becomes empty at the end

Audio meaning:

- success = good tone
- any mismatch = error tone

The serial log on the Pico side is the main proof:

- field values must stay stable before `ACK`
- queue head must advance only on `ACK`
- final `READ_STATUS` must report empty.
