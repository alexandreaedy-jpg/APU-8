# V2B Step3 Validated

Date: 2026-05-18

## Scope

Validated base:

- `P1`
- `P2`
- `TRI`
- `NOISE`
- live DIN MIDI over NES expansion port

Snapshot dirs:

- `Project-V2-A\v2b_step3_p1p2trinoise_edge_midi`
- `arduino\PicoNesV2B_Step3P1P2TriNoiseEdgeMidi`

Stable Pico revision:

- `FW=t25k-v2b-perf`

Stable ROM:

- `Project-V2-A\v2b_step3_p1p2trinoise_edge_midi\V2_MIDI_IN_P1P2.nes`

## What Was Finally Fixed

The decisive improvement did not come from a new musical mapping. It came from removing transport-side overhead that had become parasitic at full-lane tempos:

- Pico `/OE2` sniff disabled by default
- heartbeat burst disabled
- `applyOpcode()` no longer recomputes unchanged `OUT`
- ROM `BUS_READ_DELAY` reduced to `1`
- ROM vibrato/pitch maintenance skipped when no active LFO/glide work is pending

## Result

Observed result after these changes:

- `P1/P2/TRI/NOISE` hold together musically
- high tempo remains usable
- no instrument substitution
- no catastrophic queue collapse reported

This step is now the rollback-safe full-lane note base for future `DMC` work.
