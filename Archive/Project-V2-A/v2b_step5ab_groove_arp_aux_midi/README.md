# V2B Step5AB Groove AUX/ARP ROM

Base: Step5AA DMC one-shot AUX.

Objectif: tester un correctif groove sous full-lane en gardant Step5AA comme rollback-safe.

Changements par rapport a Step5AA:

- `SERVICE_AUDIO_SLICE=2` pour moins affamer LFO/enveloppes pendant les bursts transport.
- Protocole inchange: P1/P2 lanes dediees, TRI/NOISE/DMC via `AUX` type.

Pico associe:

- `arduino\PicoNesV2B_Step5ABGrooveAuxArpMidi`
- short-command target: `midi-v2b6`
