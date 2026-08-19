# V2B Step1 P1/P2 Validated

Date: 2026-05-18

## Summary

The first V2B pulse-only transport is now validated on hardware for live MIDI on:

- Pulse 1
- Pulse 2

Validated outcome reported after deploying the matching Pico + ROM pair:

- high BPM works
- `P1 + P2` together works
- no added latency
- no groove break
- no missing notes

Reference winning log:

- `C:\Users\mto1\Documents\NES_DEV\STEP2B1-Midi_test2.txt`

## Frozen Snapshot

ROM snapshot:

- `Project-V2-A\v2b_step1_p1p2_edge_midi`

Pico snapshot:

- `arduino\PicoNesV2B_Step1P1P2EdgeMidi`

Rebuild target:

- `.\tools\v2.cmd deploy midi-v2b1`

## Protocol Notes

Firmware tag:

- `FW=t21-v2b-pulse`

Transport shape:

- dedicated opcodes for `STATUS`
- dedicated read phases for `P1` low/high
- dedicated read phases for `P2` low/high
- dedicated `ACK_P1`
- dedicated `ACK_P2`

Design intent:

- keep note edges cheap
- avoid the old generic `reg/value/ext` overhead
- commit P1 and P2 from the same status pass

Key log markers from the winning run:

- `FW=t21-v2b-pulse`
- `Q` returns to `0` and stays low during play
- `QH=18`
- `OVF=0`
- `MF` and `ACK` stay matched or off by only `1` transiently
- `MX=0`
- `MB=0`

## Rule

Do not modify this frozen snapshot when experimenting with `TRI`, `NOISE`, `DMC`, or control-state expansion.
