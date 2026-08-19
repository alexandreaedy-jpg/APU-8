# V2B Step5 Control Lane Plan

Date: 2026-05-18

## Goal

Add a first slow `control lane` on top of the validated step4 note transport:

- keep `P1/P2/TRI/NOISE/DMC` note/sample transport intact
- add coalesced `ADSR + duty + LFO` controls
- never let slow controls behave like note edges

## Transport Rule

- `P1/P2` stay native pulse lanes
- `TRI/NOISE/DMC` keep using `AUX`
- controls also use `AUX`, but only when no note/sample aux event is pending
- controls are replaceable and coalesced

## AUX Control Encoding

- `0x2n` = attack
- `0x3n` = decay
- `0x4n` = sustain
- `0x5n` = release
- `0x6n` = duty
- `0x7n` = LFO depth
- `0x9n` = LFO rate

This stays disjoint from:

- `NOISE` compact note forms (`0x0n` / `0x8n`)
- `DMC` compact trigger forms (`0x1n`)

## MIDI Mapping

Channel `16` is the global control lane:

- `CC20` = attack
- `CC21` = decay
- `CC22` = sustain
- `CC23` = release
- `CC24` = duty
- `CC25` = LFO depth
- `CC26` = LFO rate

## Scope

This step is intentionally limited to:

- global control edits
- coalesced slow control transport
- no per-voice control targeting yet
- no macro/preset lane yet
