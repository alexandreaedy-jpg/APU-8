# V2B Step5AL Linear Volume + Wave Release ROM

Base: Step5AK volume curve.

Objectif: finaliser les controles panel/effects.

Changements par rapport a Step5AK:

- Volume panel revenu en courbe lineaire stricte `0..1023 -> 0..15`.
- Release maximum rallonge par rapport a Step5AK.
- Le controle panel `MCP3008 CH7` reste `Arp Time` quand ARP est ON.
- Quand ARP est OFF, `MCP3008 CH7` ne controle plus `LFO Delay`; il controle `LFO Wave`.
- Ordre waveform: `Sine > Tri > Saw > Square`.
- MIDI `CC34`, anciennement delay, devient aussi un alias waveform.
- MIDI `CC35` reste waveform avec le meme ordre.

Pico associe:

- `arduino\PicoNesV2B_Step5ALLinearVolumeWaveRelease`
- firmware: `FW=t67-v2b-step5al-linvolwave`
- short-command target: `midi-v2b16`
