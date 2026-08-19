# V2B Step5AN Log Slider Compensation

Date: 2026-05-27

Base: Step5AM slider diag / waveform / anti-pop.

Objectif:

- Corriger la course des sliders 10k V/A/D/R identifies comme log/audio dans `Log_DiagSliders02.txt`.
- Garder les rotatifs inchanges.
- Conserver la stabilite Step5AM, le correctif waveform CH7 et le duck anti-pop pulse.

Changements:

- Nouveau sketch Pico: `arduino\PicoNesV2B_Step5ANLogSliderComp`.
- Nouveau firmware: `FW=t69-v2b-step5an-logslider`.
- Nouveau target court: `midi-v2b18`.
- `panelSliderAntiHotFromRaw()` devient `panelSliderLogCompFromRaw()`.
- La table inverse-log utilise des seuils calibres pour redonner de la marge dans la zone haute du MCP3008:
  `0,191,356,496,613,710,789,852,902,940,967,987,1000,1010,1018,1022`.
- `PR=` reste actif dans le heartbeat pour comparer raw et `PV=`.

ROM:

- ROM copiee depuis Step5AM, moteur audio inchange.
- Correctif TRI/DMC conserve: TRI ne doit pas ecrire `$4015`.
