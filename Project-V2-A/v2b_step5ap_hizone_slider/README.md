# V2B Step5AP Hi-Zone Slider ROM

Base: Step5AO slider trim / long env.

Objectif: exploiter plus finement la zone haute des sliders log V/A/D/R avec hysteresis.

Changements par rapport a Step5AO:

- Pico: remplace la table simple par `panelSliderHiZoneStable()`.
- Pico: seuils tres detailles de `976` a `1023` pour exploiter la courbe log mesuree.
- Pico: hysteresis entree/sortie pour limiter le jitter dans `PV=` quand `PR=` bouge de 1 ou 2 points.
- Pico: `Volume=F` reste atteignable quand le raw MCP est au plafond (`1023`).
- Pico: `PR=` reste force dans le heartbeat pour verifier raw et quantification apres test.
- Pico: waveform conserve le correctif Step5AM: `CH7` en mode ARP OFF peut agir meme sans destination LFO selectionnee.
- ROM: inchangee par rapport a Step5AO, decay/release restent rallonges d'environ 70%.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5APHiZoneSlider`
- firmware: `FW=t71-v2b-step5ap-hizone`
- short-command target: `midi-v2b20`
