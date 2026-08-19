# V2B Step5AO Slider Trim Long Env ROM

Base: Step5AN log slider compensation.

Objectif: durcir encore la zone haute des sliders V/A/D/R, conserver le volume max, et rallonger decay/release.

Changements par rapport a Step5AN:

- Pico: table `panelSliderLogCompFromRaw()` plus resistante dans le haut pour que `850..1000` ne donne plus trop vite une valeur forte.
- Pico: `Volume=F` reste atteignable quand le raw MCP est au plafond (`1023`).
- Pico: `PR=` reste force dans le heartbeat pour verifier raw et quantification apres test.
- Pico: waveform conserve le correctif Step5AM: `CH7` en mode ARP OFF peut agir meme sans destination LFO selectionnee.
- ROM: decay et release rallonges d'environ 70% a forte valeur.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5AOSliderTrimLongEnv`
- firmware: `FW=t70-v2b-step5ao-slidertrim`
- short-command target: `midi-v2b19`
