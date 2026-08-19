# V2B Step5AQ Longer Env

Date: 2026-05-27

Base: Step5AP hi-zone slider.

Objectif:

- Conserver la lecture exploitable des sliders obtenue avec correction hardware.
- Augmenter encore le maximum du decay et du release de 50%.

Changements:

- Nouveau sketch Pico: `arduino\PicoNesV2B_Step5AQLongerEnv`.
- Nouveau firmware: `FW=t72-v2b-step5aq-longenv`.
- Nouveau target court: `midi-v2b21`.
- Pico: mapping sliders AP inchange, `panelSliderHiZoneStable()` conserve.
- ROM: table decay max `37` -> `56`.
- ROM: release `3.4x` -> `5.1x`, soit environ +50%.
- Correctif TRI/DMC conserve: TRI ne doit pas ecrire `$4015`.
