# V2B Step5AN Log Slider Compensation ROM

Base: Step5AM slider diag / waveform / anti-pop.

Objectif: compenser les sliders 10k de type log/audio sur V/A/D/R sans toucher aux rotatifs ni au moteur note stable.

Changements par rapport a Step5AM:

- Pico: V/A/D/R utilisent `panelSliderLogCompFromRaw()`, une table inverse-log calibree avec `Log_DiagSliders02.txt`.
- Pico: la table etale fortement la zone haute du raw MCP pour eviter que `850..1020` soit deja interprete comme quasi-max.
- Pico: `PR=` reste force dans le heartbeat pour verifier raw et quantification apres test.
- Pico: waveform conserve le correctif Step5AM: `CH7` en mode ARP OFF peut agir meme sans destination LFO selectionnee.
- ROM: inchangee par rapport a Step5AM.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5ANLogSliderComp`
- firmware: `FW=t69-v2b-step5an-logslider`
- short-command target: `midi-v2b18`
