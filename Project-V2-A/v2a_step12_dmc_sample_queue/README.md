# V2_A Step 12 - DMC sample queue

Objectif: valider le declenchement DMC via le transport Proto1, en restant sur le Pico seul et l'expansion port.

Registres transportes:

- `0x0E`: note sample, mappee vers kick/snare/rim/voice.
- `0x0F`: gate sample.
- `0x04`: trigger/retrigger.

Mapping de test:

- `36-37`: kick.
- `38-40`: snare.
- `41`: rim.
- `46`: voice.

La ROM lie explicitement `Project-V2-A/dmc_samples.s` pour que les adresses DMC `$C0/$CE/$D6/$DA` pointent vers les vrais samples dans le segment `SAMPLES`.
