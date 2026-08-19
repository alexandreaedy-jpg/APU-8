# V2B Step5AF Noise Bound + P2 Glide ROM

Base: Step5AE hard realtime notes.

Objectif: garder les notes prioritaires et non destructives sur P1/P2/TRI, eviter que NOISE sature l'AUX en full-lane, et retablir le glide P2.

Changements par rapport a Step5AE:

- Meme ROM que Step5AB: `SERVICE_AUDIO_SLICE=2`.
- Protocole inchange: P1/P2 lanes dediees, TRI/NOISE/DMC via `AUX` type.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.
- P1/P2/TRI restent exactes: pas de compression musicale sur ces voix.
- NOISE garde jusqu'a 8 evenements en attente, puis remplace le dernier etat NOISE non transmis au lieu de laisser la file monter vers l'overflow.
- Le heartbeat ajoute `ND=` pour compter ces compressions NOISE volontaires.
- Les controles restent latest-value-wins, injectes un peu plus prudemment (`12 ms`, `Q<=2`) pour proteger le groove.
- Le glide P2 est retabli cote ROM avec le meme pas court que P1.

Pico associe:

- `arduino\PicoNesV2B_Step5AFNoiseBoundP2Glide`
- short-command target: `midi-v2b10`
