# V2 Step 5 - First real musical control

This step leaves the synthetic queue probe and moves to a real audible control.

## Pico firmware

- unlocks safely
- maintains a live queue of logical events for `DUTY` (`reg_id=0x0`)
- generates duty values in the sequence:
  - `00 01 02 03 02 01`
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
- keeps a steady pulse note playing
- polls the queue
- applies each `DUTY` event to pulse 1

Expected result:

- audible duty/timbre changes every few hundred milliseconds
- no protocol stalls
- Pico log should show `enqueue`, `ACK pop`, and queue depth moving

If the queue ever overflows, Pico sets the overflow bit in `READ_STATUS`.
