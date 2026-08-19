# V2 Step 10 - TRI note / gate / trigger queue

This step keeps the V2 Proto1 transport unchanged and validates the first
non-pulse musical lane on the expansion-port path.

## Goal

Prove that the same Pico queue can now drive:

- `TRI note`
- `TRI gate`
- `TRI trigger`

without going back to the old controller-port chain.

## Pico firmware

- unlocks safely
- seeds one initial triangle note
- emits a short looping sequence with:
  - distinct triangle pitches
  - short silences
  - same-note retrigs

## NES ROM

- speaks the same Proto1 phases as Steps 7 to 9
- drains a few queued events per loop
- applies the events only to the triangle channel
- keeps the rendering simple so the test answers just one question:
  "does triangle note/gate/trigger travel correctly on V2?"

## Expected result

- several clearly distinct triangle notes
- short silences when `gate=0`
- same-note re-articulation when only `trigger` changes
- no transport stall

This step does **not** add `LFO` yet.
