# V2B Step5AD Exact MIDI / Panel Lock ROM

Base: Step5AB groove AUX/ARP.

Objectif: revenir a une lecture MIDI exacte apres l'echec Step5AC, en gardant le panel hors du transport pendant une prise MIDI active.

Changements par rapport a Step5AB:

- Meme ROM que Step5AB: `SERVICE_AUDIO_SLICE=2`.
- Protocole inchange: P1/P2 lanes dediees, TRI/NOISE/DMC via `AUX` type.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5ADExactMidiPanelLock`
- short-command target: `midi-v2b8`
