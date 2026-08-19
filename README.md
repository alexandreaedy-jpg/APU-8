# APU-8

APU-8 is an experimental MIDI-controlled NES instrument project.

The repository now exposes the active V2 expansion-port build first, and keeps older iterations in `Archive/`.

## Current active target

- Current ROM source: `Project-V2-A\v2b_step5ar_flat_env_pitch`
- Current ROM output: `Project-V2-A\v2b_step5ar_flat_env_pitch\V2_MIDI_IN_P1P2.nes`
- Current Pico sketch: `arduino\PicoNesV2B_Step5ARFlatEnvPitch`
- Current Pico firmware string in the sketch: `FW=t84-v2b-step5bb-groovejitter`
- Last fully documented accepted milestone: `FW=t79-v2b-step5aw-wavefix`

Quick commands:

```powershell
.\tools\v2.cmd status midi
.\tools\v2.cmd build-rom midi
.\tools\v2.cmd build-pico midi
.\tools\v2.cmd deploy midi
```

For a direct overview, start with [CURRENT_TARGETS.md](CURRENT_TARGETS.md).

## Repository layout

```text
APU-8/
|-- Project-V2-A/                     # current ROM source tree
|   `-- v2b_step5ar_flat_env_pitch/
|-- arduino/                          # current Pico sketch + diagnostics
|   |-- PicoNesV2B_Step5ARFlatEnvPitch/
|   |-- PicoMcp3008RawDiag/
|   `-- PicoPanelCtrlFullDiag/
|-- Archive/                          # archived ROM/sketch history and legacy branches
|   |-- Project-V2-A/
|   |-- arduino/
|   `-- legacy-root/
|-- hardware/                         # KiCad / PCB work
|-- max-for-live/                     # control tooling
|-- milestones/                       # frozen milestone snapshots
`-- tools/                            # build / flash shortcuts
```

## What moved to Archive

- Older `Project-V2-A` step folders
- Older Arduino Nano / Pico sketches
- Legacy controller-port ROM branches and side experiments

The build shortcuts were kept alive:

- `midi` now targets the current active build
- `midi-v2a` points to the archived V2A baseline
- `midi-v2b1` to `midi-v2b21` still point to their archived step folders
- `midi-v2b22` explicitly points to the current Step5AR folder

## Important files

- [CURRENT_TARGETS.md](CURRENT_TARGETS.md)
- [PROJECT_STATE.md](PROJECT_STATE.md)
- [SHORT_COMMANDS.md](SHORT_COMMANDS.md)
- [Project-V2-A/V2B_STEP5AR_FLAT_ENV_PITCH_2026-05-28.md](Project-V2-A/V2B_STEP5AR_FLAT_ENV_PITCH_2026-05-28.md)
- [hardware/APU8_V2B_KICAD/README.md](hardware/APU8_V2B_KICAD/README.md)

## Notes

- The active ROM folder is still named `v2b_step5ar_flat_env_pitch`, but the paired Pico sketch continued past the original Step5AR note and now reports `t84-v2b-step5bb-groovejitter`.
- `PROJECT_STATE.md` remains the long-form engineering journal.
- `Archive/` keeps history accessible without leaving the root crowded.
