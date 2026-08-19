# V2B Step5 Control Lane Validated

Date: 2026-05-19

## Scope

Validated base:

- `P1`
- `P2`
- `TRI`
- `NOISE`
- `DMC`
- per-voice controls
- clocked arp
- ultra-short automatic mono glide

Snapshot dirs:

- `Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi`
- `arduino\PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi`

Stable Pico revision:

- `FW=t30b-v2b-step5b-safe`

Stable ROM:

- `Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi\V2_MIDI_IN_P1P2.nes`

## Final MIDI Map

Voice channels:

- `CH11 = DMC`
- `CH12 = P1`
- `CH13 = P2`
- `CH14 = TRI`
- `CH15 = NOISE`
- `CH16 = global fanout`

Validated control map:

- `CC20 = attack`
- `CC21 = decay`
- `CC22 = volume`
- `CC23 = release`
- `CC24 = duty / noise timbre`
- `CC25 = pitch LFO depth`
- `CC26 = LFO rate`
- `CC27 = arp on/off`
- `CC28 = arp division`
- `CC29 = duty LFO depth`
- `CC30 = amp LFO depth`
- `CC34 = LFO delay`
- `CC35 = LFO waveform`
- `CC36 = DMC pitch`

Waveform map for `CC35`:

- `0..31 = triangle`
- `32..63 = saw`
- `64..95 = sine`
- `96..127 = square`

Arp division map for `CC28`:

- `0..31 = 1/4`
- `32..63 = 1/8`
- `64..95 = 1/16`
- `96..127 = 1/32`

## What Was Finally Fixed

This step only became stable after separating three concerns:

- keep the proven edge transport for notes and samples
- move slow controls to a compact coalesced voice-control lane
- keep glide purely ROM-side so it costs nothing on the NES bus

Important late fixes that define the frozen result:

- `CC32` was abandoned because it behaved like a problematic controller slot in the DAW/controller path
- `LFO delay/wave/DMC pitch` were remapped to `CC34/35/36`
- `TRI` remains outside envelope and amp control by design
- `LFO delay` now behaves correctly
- automatic glide was simplified and hardened into a very short near-instant mono glide
- the unsafe bend/glide transport experiment was removed to keep note lanes reliable

## Validated Musical Behavior

Observed accepted result:

- full lane remains musical
- `ARP` is OK
- `LFO rate` and `LFO delay` are OK
- `P1/P2/TRI/NOISE/DMC` all remain usable together
- ultra-short automatic glide is accepted
- no new transport regression was kept in the frozen build

Per-voice behavior:

- `P1/P2`:
  - ADSR-style controls
  - duty
  - pitch LFO
  - duty LFO
  - amp LFO
  - ultra-short auto glide when `ARP` is off
- `TRI`:
  - pitch LFO only
  - no envelope/amp lane
  - ultra-short auto glide when `ARP` is off
- `NOISE`:
  - attack/decay/volume/release
  - timbre via `duty`
  - amp LFO
- `DMC`:
  - sample triggers on `CH11`
  - sample pitch on `CC36`

## Explicit Non-Goals In Frozen Step

These are not part of the frozen validated step:

- pitch bend transport
- long expressive portamento
- extra control packing beyond the current compact lane

Those can be revisited later, but should not be reintroduced into this snapshot casually.

## Rule

Do not modify this frozen snapshot directly when experimenting with future modulation expansion or new transport ideas.
