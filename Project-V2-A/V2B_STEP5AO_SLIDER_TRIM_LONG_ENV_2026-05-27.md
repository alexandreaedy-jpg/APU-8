# V2B Step5AO Slider Trim Long Env

Date: 2026-05-27

Base: Step5AN log slider compensation.

Objectif:

- Redonner encore plus de marge aux sliders V/A/D/R dans la zone haute.
- Garder le maximum du volume atteignable.
- Augmenter la longueur maximale du decay et du release d'environ 70%.

Changements Pico:

- Nouveau sketch: `arduino\PicoNesV2B_Step5AOSliderTrimLongEnv`.
- Nouveau firmware: `FW=t70-v2b-step5ao-slidertrim`.
- Nouveau target court: `midi-v2b19`.
- Table `panelSliderLogCompFromRaw()` resserree:
  `0,256,512,700,820,900,950,980,995,1004,1010,1014,1017,1019,1021,1023`.
- `Volume=F` est conserve lorsque le raw MCP atteint `1023`.
- `PR=` reste actif dans le heartbeat pour verifier raw et `PV=`.

Changements ROM:

- `env_decay_duration()` rallonge la table de decay, jusqu'a `37` ticks au lieu de `22`.
- `env_release_duration()` passe de `2x` a `3.4x`, soit environ `+70%`.
- Correctif TRI/DMC conserve: TRI ne doit pas ecrire `$4015`.
