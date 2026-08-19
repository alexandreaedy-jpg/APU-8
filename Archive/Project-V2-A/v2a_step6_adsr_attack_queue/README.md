# V2 Step 6 - ADSR_A live queue

This step keeps the exact same transport as Step 5, but switches to a second
logical register that is easier to hear musically than `DUTY`.

## Pico firmware

- unlocks safely
- maintains a live queue of logical events for `ADSR_A` (`reg_id=0x1`)
- generates attack values in the sequence:
  - `01 04 09 0F 09 04`
- supports:
  - `READ_STATUS`
  - `READ_REG_ID`
  - `READ_VALUE_LO`
  - `READ_VALUE_HI`
  - `ACK_AND_NEXT`

Status bits:

- `D0` = `PENDING`
- `D1` = `OVERFLOW`
- `D4` = `VALID`

## NES ROM

- unlocks the transport
- polls one queued control event before each note
- applies `ADSR_A` as a simple software attack time
- retriggers the same note forever so each attack shape is easy to compare

Expected result:

- clearly different note attacks from snappy to slow swell
- no transport stall
- Pico log should show `enqueue`, `ACK pop`, and low queue depth
