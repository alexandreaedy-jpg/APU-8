# V2B Step5AQ Longer Env ROM

Base: Step5AP hi-zone slider.

Objectif: conserver la lecture hi-zone des sliders et rallonger encore le maximum decay/release.

Changements par rapport a Step5AP:

- Pico: lecture sliders inchangee, `panelSliderHiZoneStable()` conservee.
- Pico: `PR=` reste force dans le heartbeat pour verifier raw et quantification apres test.
- ROM: decay maximum augmente de `37` a `56` ticks.
- ROM: release maximum augmente d'environ 50% par rapport a Step5AP.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5AQLongerEnv`
- firmware: `FW=t72-v2b-step5aq-longenv`
- short-command target: `midi-v2b21`
