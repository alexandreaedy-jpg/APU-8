# V2 Step 8 - P1 notes + duty + ADSR on one queue

This step keeps the exact same V2 Proto1 transport discipline as Step 7, but
reintroduces slow musical controls on top of the `P1 note / gate / trigger`
lane.

## Goal

Prove that the V2 queue can carry both:

- urgent musical events:
  - `P1 note`
  - `P1 gate`
  - `P1 trigger`
- slow control events:
  - `duty`
  - `ADSR`

without going back to the controller-port jitter problems that blocked the old
architecture.

## Strategy

- one voice only: `P1`
- clearly different note pitches
- clearly different pulse `duty`
- clearly different attack shapes
- `D/S/R` remain valid and seeded, but only `attack` is actively varied here
  because it is the easiest ADSR dimension to judge quickly

## Expected result

- several distinct notes
- clear timbre changes
- clear attack changes
- no transport stall

This is the first real “notes + controls coexist” proof on the V2 Pico path.
