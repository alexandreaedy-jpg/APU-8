# V2B Step5P Panel CC Isolation - 2026-05-21

## Purpose

During physical panel debug, Ableton/MIDI CC automation can conflict with the MCP3008 panel controls. This revision makes the panel the only live source for controls that now exist physically.

## Firmware

- Pico firmware tag: `t45-v2b-step5p-panelcc`

## MIDI CC Ignored During Panel Debug

Incoming MIDI CC messages are ignored for:

- `CC20` attack
- `CC21` decay
- `CC22` volume
- `CC23` release
- `CC24` duty / noise timbre
- `CC25` pitch LFO depth
- `CC26` LFO rate
- `CC27` arp on/off
- `CC28` arp division
- `CC29` duty LFO depth
- `CC30` amp LFO depth

The MIDI parser still keeps notes, clock, all-notes-off, and non-panel CC available.

## CC Still Available

- `CC34` LFO delay
- `CC35` LFO waveform
- `CC36` DMC pitch remains ignored because the current profile is no-DMC

## Expected Result

Panel movements should no longer be overwritten by DAW envelopes or stale MIDI CC clips while calibrating the MCP3008 controls.
