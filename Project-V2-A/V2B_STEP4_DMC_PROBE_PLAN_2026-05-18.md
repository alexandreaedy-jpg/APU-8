# V2B Step4 DMC Probe Plan

Date: 2026-05-18

## Goal

Add a first `DMC` path without regressing the validated `P1/P2/TRI/NOISE` transport.

This step is intentionally a probe, not a final `DMC` feature-complete design.

## Locked Scope

- MIDI channel `11` = `DMC`
- `DMC` is trigger-only in this step
- note-off on channel `11` is ignored
- `P1/P2` remain native lanes
- `TRI/NOISE` remain on `AUX`
- `DMC` joins the `AUX` arbitration as a compact one-shot event

## Transport Choice

The first safe `DMC` probe keeps the existing `STATUS + P1 + P2 + AUX` transport.

No extra opcode is added.

Instead:

- `STATUS_AUX_IS_TRI = 1` still means `TRI`
- `STATUS_AUX_IS_TRI = 0` means either `NOISE` or `DMC`
- `NOISE` keeps its existing compact patterns:
  - high nibble `0x0`
  - or high nibble `0x8`
- `DMC` uses reserved compact trigger values:
  - `0x10` = kick
  - `0x11` = snare
  - `0x12` = rim
  - `0x13` = clap
  - `0x14` = hat

So `NOISE` and `DMC` stay disjoint without borrowing a pulse lane and without reusing a validity bit as voice type.

## MIDI Mapping

Recommended first mapping:

- notes `36-37` -> kick
- notes `38-40` -> snare
- note `41` -> rim
- notes `39` or `46` -> clap
- notes `42` or `44` -> hat

Unknown notes on channel `11` are ignored.

## Success Criteria

- `DMC` solo triggers the expected samples
- `P1/P2/TRI/NOISE` do not change instrument or behavior
- mixed passages stay musically usable
- if failure appears, it should be backlog/priority related, not identity corruption

## Not In Scope Yet

- sustained `DMC` note semantics
- velocity mapping
- loop control
- sample bank switching
- unified `NOISE/DMC` drum architecture
