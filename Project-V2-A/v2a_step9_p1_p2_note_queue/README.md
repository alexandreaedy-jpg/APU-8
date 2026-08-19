# V2 Step 9 - P1 + P2 note / gate / trigger queue

This step keeps the V2 Proto1 transport unchanged and extends the musical lane
from one pulse voice to two.

## Goal

Prove that the same Pico queue can now drive both:

- `P1 note`
- `P1 gate`
- `P1 trigger`
- `P2 note`
- `P2 gate`
- `P2 trigger`

without falling back to the old controller-port architecture.

## Pico firmware

- unlocks safely
- seeds both pulse voices
- emits short bursts that alternate:
  - two-note chords
  - `P1` only
  - `P2` only
  - same-note retrigs

## NES ROM

- speaks the same Proto1 phases as Steps 7 and 8
- drains the queue a few events at a time
- applies events independently to `PULSE1` and `PULSE2`
- keeps fixed timbres and fixed volume so the test answers only:
  "does the second pulse note lane work on V2?"

## Expected result

- audible chords at some points
- single-note passages on `P1` or `P2`
- short silences when one or both gates drop
- no transport stall

This step does **not** add new control dimensions.
It is only the second-voice extension of the validated Step 7/8 path.
