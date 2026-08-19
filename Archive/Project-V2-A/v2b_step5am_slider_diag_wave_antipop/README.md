# V2B Step5AM Slider Diag Wave Anti-Pop ROM

Base: Step5AL linear volume / wave / release.

Objectif: corriger les sliders V/A/D/R trop sensibles en bas de course, diagnostiquer le raw MCP, fiabiliser waveform, et reduire encore les clics pulse.

Changements par rapport a Step5AL:

- Pico: `PR=` force dans le heartbeat pour afficher le raw filtre des 8 controles MCP3008.
- Pico: V/A/D/R utilisent une courbe provisoire `panelSliderAntiHotFromRaw()` pour redonner de la marge si le raw ne sature pas totalement.
- Pico: waveform ne depend plus du selecteur LFO Pitch/Duty/Amp; `CH7` en mode ARP OFF peut agir meme sans destination LFO selectionnee.
- ROM: le duck autour des ecritures pulse high timer `$4003/$4007` dure 2 ticks audio pour masquer davantage les resets de phase.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5AMSliderDiagWaveAntiPop`
- firmware: `FW=t68-v2b-step5am-sliderdiag`
- short-command target: `midi-v2b17`
