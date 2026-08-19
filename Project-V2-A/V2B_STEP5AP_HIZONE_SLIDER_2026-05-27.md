# V2B Step5AP Hi-Zone Slider

Date: 2026-05-27

Base: Step5AO slider trim / long env.

Objectif:

- Mieux exploiter la courbe log des sliders V/A/D/R au lieu de l'ecraser avec une table trop grossiere.
- Detail fort dans la zone `976..1023`, observee dans `Log_DiagSliders04.txt`.
- Ajouter de l'hysteresis pour eviter le jitter lorsque le raw MCP bouge d'un ou deux points.

Changements Pico:

- Nouveau sketch: `arduino\PicoNesV2B_Step5APHiZoneSlider`.
- Nouveau firmware: `FW=t71-v2b-step5ap-hizone`.
- Nouveau target court: `midi-v2b20`.
- `panelSliderLogCompFromRaw()` remplace par:
  - `panelSliderHiZoneFromRaw()`
  - `panelSliderHiZoneStable()`
- Seuils principaux: `0,128,384,640,832,928,976,1000,1010,1015,1018,1020,1021,1022,1023,1023`.
- Seuils de sortie separes pour stabiliser les changements de niveau.
- `PR=` reste actif dans le heartbeat.

Changements ROM:

- ROM copiee depuis Step5AO, decay/release longs conserves.
- Correctif TRI/DMC conserve: TRI ne doit pas ecrire `$4015`.
