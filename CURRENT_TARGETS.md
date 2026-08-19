# Current Targets

This file is the quick entry point for the active APU-8 build.

## Active build

- ROM source folder: `ROM V2B\v2b_step5ar_flat_env_pitch`
- ROM binary: `ROM V2B\v2b_step5ar_flat_env_pitch\V2_MIDI_IN_P1P2.nes`
- Pico sketch folder: `Firmware Arduino\Firmware_PICO-V2B`
- Pico sketch file: `Firmware Arduino\Firmware_PICO-V2B\Firmware_PICO-V2B.ino`
- Current Pico firmware string: `FW=t84-v2b-step5bb-groovejitter`
- Last fully documented accepted firmware string: `FW=t79-v2b-step5aw-wavefix`

## Active helper sketches

- Raw MCP3008 diag: `Firmware Arduino\PicoMcp3008RawDiag`
- Full control-panel diag: `Firmware Arduino\PicoPanelCtrlFullDiag`

## Default commands

```powershell
.\tools\v2.cmd status midi
.\tools\v2.cmd build-rom midi
.\tools\v2.cmd build-pico midi
.\tools\v2.cmd deploy midi
```

## Historical targets

- archived V2A baseline shortcut: `midi-v2a`
- archived V2B step shortcuts: `midi-v2b1` to `midi-v2b21`
- explicit current Step5AR shortcut: `midi-v2b22`

Their source folders now live under:

- `Archive\Project-V2-A`
- `Archive\arduino`
- `Archive\legacy-root`
