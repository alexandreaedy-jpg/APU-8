# V2B Step5AC ARP Pressure ROM

Base: Step5AB groove AUX/ARP.

Objectif: tester un correctif de stabilite quand arp + DMC + toutes les voix jouent ensemble, en gardant Step5AB comme point de comparaison.

Changements par rapport a Step5AB:

- Meme ROM que Step5AB: `SERVICE_AUDIO_SLICE=2`.
- Protocole inchange: P1/P2 lanes dediees, TRI/NOISE/DMC via `AUX` type.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5ACArpPressureMidi`
- short-command target: `midi-v2b7`
