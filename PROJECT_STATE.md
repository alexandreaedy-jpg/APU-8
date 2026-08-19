# NES_DEV V2_A - Project State

## Goal

Build a reliable Pico-driven NES expansion-port control path for MIDI/control features, replacing the old Nano/controller-port V18 path where jitter, ignored notes, and LFO timing became limiting.

## Current Focus

`Step5AA DMC one-shot AUX`: keep the Step5Z typed `AUX` baseline, keep `DMC pitch` disabled, and treat DMC as a sample trigger class rather than a note/control pair competing with `TRI`.

Success criteria:
- keep the validated `step5` snapshot rebuildable
- preserve stable full-lane note transport
- preserve the stable subset of the control map
- make the LFO panel selector deterministic: `GP8=Pitch`, `GP9=Duty`, `GP15=Amp`
- reuse `MCP3008 CH7` as `Arp Time` with arp ON and `LFO Delay` with arp OFF
- ignore the panel volume slider until its hardware issue is repaired
- use active analog control locking instead of one-shot largest-delta selection
- make `Decay 0..14` percussive and `Decay 15` sustain/held
- minimize `$4003/$4007` rewrites before trying any sweep-unit anti-click experiment
- keep LFO active during arp without mute/restore control bursts
- coalesce panel/MIDI controls through a scheduler instead of injecting whole bursts
- keep note transport immediate; the Step5T short note cohort is disabled because it caused audible groove drag
- use `PicoMcp3008RawDiag` to isolate the `MCP3008 CH0` / Slider1 hardware path before recalibrating volume
- hold panel controls as latest-value-wins while the active transport queue is above the safe release threshold
- swap panel `CH0/CH1`: physical slider 1 is `Attack`, physical slider 2 is `Volume`
- keep ultra-short automatic glide only on `P1`
- prevent `TRI`/`DMC` decode collisions by typing `AUX` in the status byte; a raw `TRI` note/gate byte can no longer look like a sample trigger
- map `CH11` notes `36..61` to the 26 Amen DMC slices generated from `C:\Users\mto1\Documents\NES SAMPLES\AMENBREAK-SLIces`
- transport DMC as a single typed one-shot event, with bounded interleaving under `TRI/NOISE` pressure instead of unconditional drops
- use this as the rollback-safe baseline for future sound-design work

## Validated Milestones

- Step7: P1 note/gate/trigger OK
- Step8: P1 notes + ADSR/duty OK
- Step9: P1 + P2 OK
- Step10: TRI OK
- Step11: noise OK
- Step12: DMC samples OK
- Step13: full-lane queue eventually OK
- Step14: full-lane play OK
- Step15: music pattern OK enough
- Step22: musical LFO good
- Step23: full-lane + musical LFO accepted
- Step24: pitch engine OK
- Step25: portamento OK
- Step5B: full control lane + arp + DMC pitch + ultra-short glide accepted
- Step5C: stability profile in progress, prioritizing groove and strict voice separation

## Current MIDI Variant

ROM:
- `Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi\V2_MIDI_IN_P1P2.nes`
- deployed target: `D:\game.nes`
- frozen snapshot: `Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi\V2_MIDI_IN_P1P2.nes`

Pico sketch:
- `arduino\PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi\PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi.ino`
- MIDI RX: GP1
- upload port: COM6
- frozen snapshot: `arduino\PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi\PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi.ino`

Current Step5 baseline:
- Pico firmware tag:
  - `FW=t56-v2b-step5aa-dmconeshot`
- short-command target:
  - `midi-v2b5`
- current groove experiment:
  - `midi-v2b6`
  - `FW=t57-v2b-step5ab-groove`
  - notes: `Project-V2-A\V2B_STEP5AB_GROOVE_AUX_ARP_2026-05-26.md`
- current arp-pressure experiment:
  - `midi-v2b7`
  - `FW=t58-v2b-step5ac-arppressure`
  - notes: `Project-V2-A\V2B_STEP5AC_ARP_PRESSURE_2026-05-26.md`
- current exact-MIDI recovery experiment:
  - `midi-v2b8`
  - `FW=t59-v2b-step5ad-exact-panel-lock`
  - notes: `Project-V2-A\V2B_STEP5AD_EXACT_MIDI_PANEL_LOCK_2026-05-26.md`
- current hard realtime note experiment:
  - `midi-v2b9`
  - `FW=t60-v2b-step5ae-hardrt`
  - notes: `Project-V2-A\V2B_STEP5AE_HARD_REALTIME_NOTES_2026-05-26.md`
- current noise-bound / P2-glide experiment:
  - `midi-v2b10`
  - `FW=t61-v2b-step5af-noisebound`
  - notes: `Project-V2-A\V2B_STEP5AF_NOISE_BOUND_P2_GLIDE_2026-05-27.md`
- current stream 5-lane / event experiment:
  - `midi-v2b11`
  - `FW=t62-v2b-step5ag-stream5`
  - notes: `Project-V2-A\V2B_STEP5AG_STREAM_5LANE_EVENT_2026-05-27.md`
- current stream fast-audio guard experiment:
  - `midi-v2b12`
  - `FW=t63-v2b-step5ah-fastguard`
  - notes: `Project-V2-A\V2B_STEP5AH_STREAM_FAST_AUDIO_GUARD_2026-05-27.md`
- current expressive FX / TRI glide experiment:
  - `midi-v2b13`
  - `FW=t64-v2b-step5ai-fxtri`
  - notes: `Project-V2-A\V2B_STEP5AI_EXPRESSIVE_FX_TRI_GLIDE_2026-05-27.md`
- current poly / LFO delay / release / anti-pop experiment:
  - `midi-v2b14`
  - `FW=t65-v2b-step5aj-polydelay`
  - notes: `Project-V2-A\V2B_STEP5AJ_POLY_DELAY_RELEASE_ANTIPOP_2026-05-27.md`
- current volume curve experiment:
  - `midi-v2b15`
  - `FW=t66-v2b-step5ak-volcurve`
  - notes: `Project-V2-A\V2B_STEP5AK_VOLUME_CURVE_2026-05-27.md`
- current linear volume / waveform / release experiment:
  - `midi-v2b16`
  - `FW=t67-v2b-step5al-linvolwave`
  - notes: `Project-V2-A\V2B_STEP5AL_LINEAR_VOLUME_WAVE_RELEASE_2026-05-27.md`
- current slider diag / waveform / anti-pop experiment:
  - `midi-v2b17`
  - `FW=t68-v2b-step5am-sliderdiag`
  - notes: `Project-V2-A\V2B_STEP5AM_SLIDER_DIAG_WAVE_ANTIPOP_2026-05-27.md`
- current log-slider compensation experiment:
  - `midi-v2b18`
  - `FW=t69-v2b-step5an-logslider`
  - notes: `Project-V2-A\V2B_STEP5AN_LOG_SLIDER_COMP_2026-05-27.md`
- current slider trim / long env experiment:
  - `midi-v2b19`
  - `FW=t70-v2b-step5ao-slidertrim`
  - notes: `Project-V2-A\V2B_STEP5AO_SLIDER_TRIM_LONG_ENV_2026-05-27.md`
- current hi-zone slider experiment:
  - `midi-v2b20`
  - `FW=t71-v2b-step5ap-hizone`
  - notes: `Project-V2-A\V2B_STEP5AP_HIZONE_SLIDER_2026-05-27.md`
- current longer envelope experiment:
  - `midi-v2b21`
  - `FW=t72-v2b-step5aq-longenv`
  - notes: `Project-V2-A\V2B_STEP5AQ_LONGER_ENV_2026-05-27.md`
- current panel envelope cleanup experiment:
  - `midi-v2b22`
  - final accepted ROM base: `Project-V2-A\v2b_step5ar_flat_env_pitch\V2_MIDI_IN_P1P2.nes`
  - final accepted Pico firmware: `FW=t79-v2b-step5aw-wavefix`
  - notes: `Project-V2-A\V2B_STEP5AR_FLAT_ENV_PITCH_2026-05-28.md`
- channel map:
  - `CH11 = DMC Amen trigger-only, notes 36..61`
  - `CH12 = P1`
  - `CH13 = P2`
  - `CH14 = TRI`
  - `CH15 = NOISE`
  - `CH16 = global fanout`
- validated control map:
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
  - `CC34 LFO delay`
  - `CC35 LFO waveform`
  - `CC36 DMC pitch` ignored in Step5AA
- design constraints:
  - `TRI` stays outside envelope and amp control
  - `NOISE` is panel-safe: volume, release and timbre only until the control path is proven stable
  - `DMC` is trigger-only in Step5AA; no DMC pitch/control traffic is active
  - `AUX` can carry `TRI`, `NOISE`, or `DMC`, but the voice type is explicit in status bits `D3..D4`, never inferred from payload bytes
  - Step5D keeps exact arp ordering and increases per-voice queue headroom
  - Step5E uses `D4` as data, not as a permanent valid bit
  - Step5E status `D4` means `AUX_FULL`; `NOISE` note/gate can use a one-read short packet
  - Step5E ROM interleaves audio updates during transport bursts so LFO/envelopes do not starve
  - Step5F makes `NOISE` CC meta/value pairs outrank note bursts on `AUX`
  - Step5F maps `CC24` on `NOISE` as 4-bit timbre: mode bit plus period offset
  - Step5G ignores repeated `CC27 arp enable` states so ON/ON and OFF/OFF do not reset the arp
  - Step5G ignores repeated `CC28 arp division` buckets so the arp counter is not reset by duplicate messages
  - Step5H adds MCP3008 panel controls for V/A/D/R, LFO depth/rate, duty/timbre, and arp time
  - Step5H uses `GP16/17/18/19` for MCP3008 SPI and `PC=` in heartbeat for panel-applied control count
  - Step5H panel is latch/filter based: stable pots produce no repeated NES traffic
  - Step5I adds selector debounce and prevents a voice/LFO selector change from dumping all current pot values to the new target
  - Step5I heartbeat adds `PT=`, `PL=`, and `PV=` to debug panel voice target, LFO target, and MCP3008 quantized values
  - Step5J applies only the dominant analog channel per panel scan, latching other simultaneous changes without sending them
  - Step5J heartbeat adds `PA=` for last applied panel control and `PS=` for suppressed simultaneous analog changes
  - Step5N reads the LFO target selector in both polarities, falls back to pitch if ambiguous, and heavily softens panel attack
  - Step5O keeps the LFO target selector fixed active-low with pull-ups, softens attack further, and speeds panel filtering to 50/50
  - Step5P ignores incoming MIDI `CC20..CC30` so front-panel controls are the only live source for V/A/D/R, duty/timbre, LFO depth/rate, and arp on/time during panel debug
  - Step5Q makes the physical LFO depth selector exclusive: selecting Pitch clears Duty/Amp depth, selecting Duty clears Pitch/Amp depth, selecting Amp clears Pitch/Duty depth
  - Step5Q applies the current LFO depth immediately when the LFO target selector changes, and defaults the LFO waveform to sine in both Pico and ROM
  - Step5R maps the physical `Arp Time` pot to `LFO Delay` while arp is OFF, and back to arp division while arp is ON
  - Step5S uses a `120 ms` active-control lock for the analog panel so one moving pot/slider no longer lets small crosstalk deltas from other channels win the scan
  - Step5S uses per-control deadbands: `A/D/R=16`, `LFO depth/rate=20`, `Duty/ArpDelay=32`
  - Step5S reapplies current `LFO depth/rate/delay` after voice/LFO-target changes and after arp OFF, without dumping `A/D/R/Duty`
  - Step5S changes P1/P2/NOISE decay into a percussive decay-to-zero model; `decay=15` is the only held/sustain setting
  - Step5S reduces pulse clicks by avoiding forced high-timer writes when `$4003/$4007` high bits are unchanged
  - Step5T swaps panel `CH0/CH1`, making `CH0=Attack` and `CH1=Volume`
  - Step5T removes arp-time LFO mute/restore bursts; LFO controls remain valid while arp is active
  - Step5T adds a coalesced voice-control scheduler: notes first, then at most one `meta/value` control pair when the target lane is clear
  - Step5U disables the Step5T `1200 us` note cohort and restores immediate MIDI note exposure to protect groove
  - Step5U lets scheduled controls append behind pending notes instead of waiting for an empty target lane, improving `P2` and panel reactivity
  - Step5U rotates the scheduler voice cursor so `P2` is not permanently behind `NOISE/P1` during global or heavy panel control traffic
  - Step5V adds `arduino\PicoMcp3008RawDiag` for raw `MCP3008 CH0..CH7` diagnosis without MIDI/NES transport
  - Step5V throttles scheduled control injection to one pair every `6 ms` and only when live queue pressure is `Q<=2`
  - Step5V clamps panel-origin `Volume=0` to `1` until the `CH0` hardware path is proven reliable
  - Step5V requires `CH7` Arp Time / LFO Delay to be stable for `40 ms` before sending a new effective value
  - Step5W re-enables `CH11` DMC with one-byte full `AUX` triggers `0x40..0x5F`, reducing traffic versus the old DMC meta/value pair
  - Step5W generates `dmc_samples.s` and `dmc_samples.h` from the Amen `.dmc` folder; 26 slices fit in the `$F000..$FFFA` DMC bank
  - Step5W keeps the DMC channel bit preserved during triangle gate changes so `TRI` no longer disables an active sample
  - Step5X supersedes Step5W DMC transport: DMC now uses guarded bytes `0xFD` or `0xFE`, then `0xE0..0xEF`, preventing `TRI` note-off values from being decoded as samples
  - Step5X caps DMC queue pressure and drops DMC triggers when `TRI/NOISE` already have `AUX` backlog; heartbeat field `DD=` reports these deliberate drops
  - Step5X caps live DMC playback rate to `$0D` to reduce peak DMC DMA pressure on the ROM transport loop
  - Step5Y fixes the remaining ROM decode hole: `STATUS_AUX_IS_TRI` now always wins over DMC-looking bytes, and stale DMC meta arms are cleared by any non-DMC AUX packet
  - Step5Y adds a Pico DMC firewall: CH11 samples are dropped when musical queues or scheduled controls are already pending, with `DD=` reporting the drops
  - Step5Y slows panel-control injection to one pair every `16 ms` and only when live queues are empty, so controls no longer steal transport slots from notes
  - Step5Y increases active voice queues from `32` to `48` slots to absorb short exact-note bursts with less `OVF`
  - Step5Z replaces `STATUS_AUX_IS_TRI/AUX_FULL` with typed `AUX` status: `00=NOISE short`, `08=TRI full`, `10=DMC full`, `18=NOISE full/control`
  - Step5Z removes the last payload-shape decode path: ROM chooses `TRI/NOISE/DMC` only from status bits, not from `FD/FE/E*` values
  - Step5Z keeps `NOISE` note/gate compact for bandwidth while sending `NOISE` controls as explicitly typed full-byte `AUX`
  - Step5AA supersedes the guarded DMC pair: `CH11` now sends one typed `AUX=DMC` byte containing only the sample id `0..25`
  - Step5AA removes `$4015` writes from `update_triangle()`; only `trigger_dmc()` clears/sets the DMC enable bit, preventing TRI gate changes from restarting an already-finished DMC sample
  - Step5AA lets DMC interleave after a small bounded `AUX` deferral instead of being dropped whenever `P1/P2/TRI/NOISE` have pending note traffic
  - Step5AB tests `AUX` rotation plus `SERVICE_AUDIO_SLICE=2` for better groove under full-lane load
  - Step5AC makes arp voices latest-state-wins and drops new DMC one-shots only under heavy active-arp queue pressure
  - Step5AD abandons Step5AC's lossy musical replacement, preserves exact note transport, and locks panel writes during active MIDI playback
  - Step5AE gives notes hard priority over unsent controls, keeps CC/panel latest-value-wins, and adds note/control latency metrics
  - Step5AF caps pending `NOISE` note backlog at 8 by replacing the newest stale NOISE state, adds heartbeat `ND=`, slows control injection to `12 ms`/`Q<=2`, and restores ROM-side P2 glide
  - Step5AG replaces typed `AUX` with a stream protocol: `STATUS` bits announce `P1/P2/TRI/NOISE/EVENT`, repeated `READ_DATA` phases carry the ordered payload, and `EVENT` carries DMC + controls
  - Step5AH keeps the Step5AG stream but reduces ROM bus-read settle time, updates audio based on lane count, and lets EVENT controls inject only at `Q=0` every `24 ms`
  - Step5AI keeps Step5AH timing stable, adds an extended LFO depth curve, shortens release/decay, enables legato `TRI` glide, and allows `TRI` pitch LFO controls
  - Step5AJ lengthens Step5AI release by about 50%, re-enables `CC34 LFO delay`, adds `CH16` poly notes over `P1/P2/TRI`, and ducks pulse volume around mid-note `$4003/$4007` writes to reduce glide pops
  - Step5AK keeps Step5AJ audio/transport and changes only the panel volume pot curve, with `MCP3008 CH1` using a slow non-linear table and `Volume=0` restored
  - Step5AL restores strictly linear panel volume, lengthens release again, and replaces LFO delay control with waveform selection ordered `Sine > Tri > Saw > Square`
  - Step5AM enables raw MCP `PR=` logging, applies a provisional anti-hot curve to V/A/D/R sliders, lets CH7 waveform work without Pitch/Duty/Amp selection, and extends the pulse high-timer duck to reduce glide clicks
  - Step5AN keeps Step5AM transport/ROM and replaces the provisional V/A/D/R slider curve with an inverse-log compensation table calibrated from `Log_DiagSliders02.txt`
  - Step5AO tightens the high end of the slider compensation from `Log_DiagSliders03.txt`, keeps volume max at raw `1023`, and lengthens decay/release by about 70%
  - Step5AP keeps Step5AO ROM/envelope timing and replaces static slider mapping with hi-zone thresholds plus hysteresis for V/A/D/R
  - Step5AQ keeps Step5AP slider mapping and increases decay max from `37` to `56` ticks plus release from `3.4x` to `5.1x`
  - Step5AS keeps Step5AR's flat decay model, treats `decay=12..15` as a 100% plateau until note-off so top-end slider jitter cannot shorten held notes, uses a softer early release table, adds heartbeat `VC=` for actual retained A/D/V/R state, and keeps the P1/P2 pitch hachure mitigation
  - Step5AR final accepted state adds a fine internal envelope with final APU quantization, sweep-based pulse anti-pop on high-byte timer transitions, cleaned chained-glide behavior, stronger top-end pitch LFO depth, faster but non-folding LFO rate scaling, widened internal pitch LFO width, and a fixed CH7 waveform selector in Pico firmware (`FW=t79-v2b-step5aw-wavefix`)
  - Step5T added a short note cohort window for simultaneous voices; Step5U supersedes it and keeps the code path disabled for groove stability
  - Step5T adds ROM-side `NOISE` timbre LFO using `CTRL_LFO_DUTY_DEPTH`, with no continuous Pico transport traffic
  - Step5T heartbeat adds `CS=` for pending/injected control scheduler state and `NC=` for note cohort state
  - `NOISE` CC pairs can be promoted ahead of stale `NOISE` backlog when safe
  - `P1` glide uses a short adaptive step so small intervals remain audible
  - `NOISE` CC pairs can jump to the front of the noise queue, without splitting an active meta/value pair
  - auto glide is ROM-side, ultra short, and `P1` only
  - LFO controls are ignored while arp is active on the target voice
  - no experimental bend/glide transport is active in the frozen build

## Known Risks

- Full-lane MIDI with notes + CC + five voices may saturate again.
- If queue is high or `OVF=1`, the transport is overloaded, not necessarily musically wrong.
- If volume is low on short notes, suspect envelope/attack before transport.
- Real-time controls must be compact and replace stale pending controls instead of stacking.
- `1/32` arp can still exceed physical bandwidth under sustained extreme load; Step5D/Step5E buffer exact events instead of dropping them.
- Step5E idle logs show `D=00000`; this is expected because `D4` is no longer a constant valid bit.
- Do not use long snapshots for fast MIDI.

## Latest MIDI Result

- Step1 validated:
  - `Project-V2-A\V2B_STEP1_P1P2_VALIDATED_2026-05-18.md`
- Step3 validated:
  - `Project-V2-A\V2B_STEP3_P1P2TRINOISE_VALIDATED_2026-05-18.md`
- Step5 now validated as the current rollback-safe baseline:
  - `Project-V2-A\V2B_STEP5_CTRL_VALIDATED_2026-05-19.md`
- current frozen expressive build:
  - ROM: `Project-V2-A\v2b_step5ar_flat_env_pitch\V2_MIDI_IN_P1P2.nes`
  - Pico: `arduino\PicoNesV2B_Step5ARFlatEnvPitch\PicoNesV2B_Step5ARFlatEnvPitch.ino`
  - firmware string expected in logs: `FW=t79-v2b-step5aw-wavefix`
  - accepted behavior:
    - stable full-lane note transport
    - TRI/DMC conflict fix preserved (`update_triangle()` must not write `$4015`)
    - panel V/A/D/R usable with current slider hardware
    - waveform selector on CH7 working
    - pulse glide/transition anti-pop strongly improved
    - pitch LFO depth/rate expanded for more expressive FX
- accepted outcome of the frozen Step5 build:
  - stable full-lane musical behavior
  - `DMC` sample lane kept working
  - per-voice control lane accepted
  - `ARP` accepted
  - `LFO delay` accepted
  - ultra-short automatic glide accepted
  - unsafe bend/glide transport experiment intentionally removed from the frozen build

## Do Not Touch Unless Explicitly Asked

- validated Step14-Step25 folders
- reference ChipMaestro/NESizer folders
- old Nano/V18 branches except for reading/reference
- frozen `v2b_step1_*`, `v2b_step3_*`, and `v2b_step5_*` snapshots unless explicitly branching a new experiment

## Next Evolution Plan

1. Freeze this build as the sound-design reference and avoid touching transport/protocol unless a regression appears.
2. Finish the remaining micro-click work:
   - chained glide edge clicks
   - pulse note-start click under some retriggers
3. Improve LFO resolution only if the current waveforms still feel stepped:
   - larger internal waveform table
   - keep the current non-folding rate ceiling
4. Explore next musical features on top of the frozen base:
   - richer poly/global allocation behavior
   - more expressive arp behavior per voice
   - optional advanced modulation ideas without reopening transport timing
5. Document the final wiring and frozen deployment pair so recovery is instant after future experiments.
3. Add replaceable `STATE_*` / `CTRL_*` slots only after `P1/P2/TRI` is musically stable.
4. Keep `midi-v2b3` as the frozen rollback-safe full-lane note base.
5. Hardware-test `midi-v2b4` for `DMC` solo on channel `11`, then mixed with `P1/P2/TRI/NOISE`.
6. Hardware-test `midi-v2b5` for `CH16` control edits while notes keep running.
7. Only after stable trigger-only `DMC` + stable control lane, define whether sustained/sample-state behavior is worth adding.
