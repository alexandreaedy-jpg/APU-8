# V2 Step 7 - P1 note / gate / trigger queue

This step is the first true musical transport extension on the V2 Proto1 path.

## Goal

Keep the validated `OUT0..OUT2 + queue + ACK` transport, but prove that it can
now drive:

- `P1 note`
- `P1 gate`
- `P1 trigger`

without relying on the old controller-port note lane.

## Pico firmware

- unlocks safely
- exposes a live queue of logical events for:
  - `P1_NOTE` (`reg_id=0x8`)
  - `P1_GATE` (`reg_id=0x9`)
  - `P1_TRIG` (`reg_id=0xA`)
- sends a small looping note pattern with a few silences and same-note retrigs

## NES ROM

- unlocks the transport with a short bring-up sequence
- drains several queued events per loop
- applies them to `PULSE1` only
- keeps the rendering intentionally simple so the test answers one question:
  "do notes, gate and trigger travel correctly on V2 Proto1?"

## Expected result

- several distinct pitches, not one fixed tone
- short silences when `gate=0`
- same-note re-articulation when only `trigger` changes
- no transport stall

This step does **not** reintroduce `ADSR` or `LFO`.
Those come back only after the `P1 note/gate/trigger` lane is proven.
