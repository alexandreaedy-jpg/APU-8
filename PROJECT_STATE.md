# NES_DEV V2B - Project State

## Goal

Build a reliable Pico-driven NES expansion-port control path for MIDI/control features, replacing the old Nano/controller-port V18 path where jitter, ignored notes, and LFO timing became limiting.

## Repository Layout Note

- active ROM folder: `ROM V2B\v2b_step5ar_flat_env_pitch`
- active ROM binary: `ROM V2B\v2b_step5ar_flat_env_pitch\V2_MIDI_IN_P1P2.nes`
- active Pico sketch: `Firmware Arduino\Firmware_PICO-V2B`
- archived step folders still live under `Archive\Project-V2-A` and `Archive\arduino`

## Current Active Pair

ROM:
- `ROM V2B\v2b_step5ar_flat_env_pitch\V2_MIDI_IN_P1P2.nes`
- deployed target: `D:\game.nes`
- source folder: `ROM V2B\v2b_step5ar_flat_env_pitch`

Pico sketch:
- `Firmware Arduino\Firmware_PICO-V2B\Firmware_PICO-V2B.ino`
- MIDI RX: `GP1`
- upload port: `COM6`
- current sketch firmware string: `FW=t84-v2b-step5bb-groovejitter`
- last fully documented accepted firmware string: `FW=t79-v2b-step5aw-wavefix`
- default shortcut target: `midi`

Current ROM note kept locally:
- `ROM V2B\V2B_STEP5AR_FLAT_ENV_PITCH_2026-05-28.md`

## Historical Variants

- archived V2A baseline shortcut: `midi-v2a`
- archived V2B shortcuts: `midi-v2b1` to `midi-v2b22`
- older step notes were removed from the active ROM folder to keep the root clean
- full history remains available through `Archive\` and git history

## Channel Map

- `CH11 = DMC Amen trigger-only, notes 36..61`
- `CH12 = P1`
- `CH13 = P2`
- `CH14 = TRI`
- `CH15 = NOISE`
- `CH16 = global fanout`

## Validated Control Map

- `CC20 attack`
- `CC21 decay`
- `CC22 volume`
- `CC23 release`
- `CC24 duty / noise timbre`
- `CC25 pitch LFO depth`
- `CC26 LFO rate`
- `CC27 arp on/off`
- `CC28 arp division`
- `CC29 duty LFO depth`
- `CC30 amp LFO depth`
- `CC35 LFO waveform`

## Accepted State

- stable full-lane note transport
- TRI/DMC conflict fix preserved (`update_triangle()` must not write `$4015`)
- panel V/A/D/R usable with current slider hardware
- waveform selector on `CH7` working
- pulse glide / transition anti-pop strongly improved
- pitch LFO depth and rate expanded for more expressive FX
- DMC sample lane kept working
- per-voice control lane accepted
- arp accepted
- unsafe bend/glide transport experiment intentionally removed from the frozen build

## Known Risks

- full-lane MIDI with notes + controls + five voices can still saturate if future experiments reopen the transport budget
- if queue is high or `OVF=1`, the transport is overloaded, not necessarily musically wrong
- real-time controls must stay compact and replace stale pending controls instead of stacking
- do not use long snapshots for fast MIDI

## Do Not Touch Unless Explicitly Asked

- archived Step14-Step25 folders
- reference ChipMaestro / NESizer folders
- old Nano / V18 branches except for reading and reference
- archived `v2b_step1_*`, `v2b_step3_*`, and `v2b_step5_*` snapshots unless explicitly branching a new experiment

## Next Evolution Plan

1. Freeze this build as the sound-design reference and avoid touching transport/protocol unless a regression appears.
2. Finish the remaining micro-click work only if it is still audible on hardware.
3. Improve LFO resolution only if the current waveforms still feel stepped in musical use.
4. Explore richer poly/global behavior and more expressive per-voice arp ideas on top of the frozen base.
5. Keep wiring and deployment recovery simple so the stable pair can always be rebuilt quickly.
