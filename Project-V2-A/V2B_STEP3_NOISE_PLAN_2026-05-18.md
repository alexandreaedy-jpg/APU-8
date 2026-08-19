# V2B Step3 NOISE Plan

Date: 2026-05-18

## Goal

Add `NOISE` on top of the V2B edge-driven transport without regressing:

- validated `P1/P2`
- current `P1/P2/TRI` live behavior
- startup behavior
- groove stability under dense note traffic

## What The References Say

### NESizer

`NESizer` does not treat `NOISE` as a giant special-case transport.

- In `assigner.c`, `NOISE` note-on is essentially:
  - gate on
  - update a compact period index
- Note-off is essentially:
  - gate off
- In `apu.h`, the noise state is very small:
  - `loop`
  - `volume`
  - `period`
- In `apu.c`, `noise_update()` only mirrors:
  - volume
  - loop/tonal bit
  - 4-bit period
- `DMC` is handled separately.

Design lesson:

- `NOISE` is a compact state voice.
- `DMC/sample` is the truly separate case.

### ChipMaestro

`ChipMaestro` follows the same philosophy.

- `Channel.cpp` builds a short packet for `type == 2` (`NOISE`):
  - note-on / note-off behavior
  - constant volume byte
  - 4-bit period selection
  - optional tonal-noise bit
- `Channel.h` exposes a separate `tonalNoise` flag.
- `ChipMaestro.ino` treats tonal noise as a toggle/control, not as part of a heavy transport.

Design lesson:

- `NOISE` can stay compact.
- The only extra state worth caring about early is the tonal/loop flag.

## Conclusion

There is nothing fundamentally "huge" or "exceptional" about `NOISE` transport itself.

The mistake would be to couple `NOISE` and `DMC` too early, or to ship `NOISE` through a heavier generic state path than `P1/P2/TRI`.

For V2B, `NOISE` should be introduced as:

- one more edge-driven voice
- compact note/gate payload
- optional slow state for tonal/loop
- no DMC coupling in the first stable milestone

## Recommended Scope For Step3

Implement only:

- `P1`
- `P2`
- `TRI`
- `NOISE`
- note/gate edge transport

Do not add yet:

- `DMC` sample triggering through the same path
- replaceable control/state packets for noise modulation
- full noise envelope/LFO feature parity
- hybrid `NOISE/DMC` note interpretation

## Recommended Transport Shape

Keep the current V2B philosophy:

- fast direct path when only `P1/P2` are pending
- dynamic slot path when `TRI` or `NOISE` edges are pending
- edge queues per voice
- no global snapshot frame for note traffic

Recommended voice set:

- `VOICE_P1`
- `VOICE_P2`
- `VOICE_TRI`
- `VOICE_NOISE`

Recommended compact payload:

- `bit7 = gate`
- `bits6..0 = note/index`

Why keep it this way:

- same mental model as validated `P1/P2`
- same edge semantics as current `TRI`
- small payload cost
- easy to preserve note edges without stale replacement mistakes

## Recommended ROM Behavior

For the first `NOISE` milestone, split `NOISE` from `DMC`.

Do:

- create a dedicated `apply_noise_edge()` / `update_noise()` path
- map incoming note to a compact 16-step noise period selection
- retrigger `NOISE_HI` on note-on edges
- mute or release via gate-off
- keep constant-volume behavior initially

Do not do in the first milestone:

- auto-switch to DMC samples based on note number
- mixed `noise_or_dmc` behavior

Reason:

- refs treat `DMC` as a different subsystem
- mixing both too early makes diagnosis harder
- we first need a musically solid `NOISE` lane

## Recommended Minimal State

For step3, `NOISE` only needs:

- note / period index
- gate
- optional trigger counter

Optional next-state, but not required for first validation:

- tonal / loop flag

## Suggested Mapping Policy

Start simple and explicit:

- MIDI channel `15` stays `NOISE`
- incoming MIDI note maps to a 16-step noise period index
- first validation can use fixed non-tonal noise

Then later:

- add one slow control for `tonalNoise` / loop mode
- decide if some note ranges should become drum-style aliases

## Why This Is Safer

This keeps `NOISE` aligned with the V2B win:

- edge-first
- small payloads
- no rebuild of music from a generic register flood

And it avoids the failure pattern we already saw:

- if we make `NOISE` too generic too early, transport cost rises before we even reach `DMC`

## Success Criteria

- `P1/P2/TRI` must not regress
- `NOISE` solo must be immediate and stable
- `P1/P2 + NOISE` must stay musical
- `P1/P2/TRI + NOISE` must remain usable without obvious queue collapse
- `NOISE` must work before any `DMC` reintegration

## After Step3

Only after stable `NOISE`:

1. add tonal-noise control
2. decide the clean `DMC` strategy
3. only then consider shared drum/sample policy
