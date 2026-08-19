# V2 Step 11 - NOISE note / gate / trigger queue

This step keeps the V2 Proto1 transport unchanged and validates the hardware
noise lane before testing DMC samples.

## Goal

Prove that the same Pico queue can drive:

- `NOISE note`
- `NOISE gate`
- `NOISE trigger`

## Strategy

- use only noise notes outside the DMC sample note range
- render short hardware-noise hits
- include gate drops and same-note retriggers

## Expected result

- several distinct noise colors or pitches
- short gated silences
- repeated hits when only the trigger changes
- no transport stall

Samples/DMC are intentionally left for the next step.
