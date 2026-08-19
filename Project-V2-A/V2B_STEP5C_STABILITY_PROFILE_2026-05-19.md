# V2B Step5C Stability Profile

Date: 2026-05-19

## Summary

Step5C is a live-stability profile derived from `FW=t30b-v2b-step5b-safe`.

The goal is to keep the groove intact under heavy MIDI load, even if that means disabling a few expressive combinations that overloaded the shared `AUX` lane.

## Rules

- `P1`, `P2`, `TRI`, and `NOISE` note events stay priority traffic.
- `AUX_IS_TRI` is authoritative: a `TRI` event can never be decoded as `DMC`.
- `DMC` is disabled in the no-DMC stability profile; `CH11` is ignored.
- `CC36 DMC pitch` is ignored in this profile.
- `NOISE` does not support arp.
- `P1` keeps the ultra-short auto glide only when its arp is off.
- `P1` glide uses an adaptive short step so adjacent notes still produce an audible glide.
- `P2` and `TRI` have no auto glide in Step5C.
- LFO controls are ignored while arp is active on a voice.
- Enabling arp mutes that voice's LFO depths on the ROM side.

## MIDI Controls Kept

- `CC20 = attack`
- `CC21 = decay`
- `CC22 = volume`
- `CC23 = release`
- `CC24 = duty / noise timbre`
- `CC25 = pitch LFO depth`, only when arp is off
- `CC26 = LFO rate`, only when arp is off
- `CC27 = arp on/off`
- `CC28 = arp division`
- `CC29 = duty LFO depth`, only when arp is off
- `CC30 = amp LFO depth`, only when arp is off
- `CC34 = LFO delay`, only when arp is off
- `CC35 = LFO waveform`, only when arp is off

## Validation Targets

- No `TRI` event should trigger a DMC sample because the DMC path is inactive.
- `AUX` should carry only `TRI` or `NOISE`.
- No infinite `TRI` sustain after note-off.
- No glide on `P2` or `TRI`.
- No glide on `P1` while arp is active.
- No audible LFO while arp is active on a voice.
- `NOISE` controls remain usable without arp.
- Dense live test should prioritize stable groove over expressive modulation.
- `NOISE` CC pairs may be prioritized inside the noise queue, but active meta/value pairs must never be split.
