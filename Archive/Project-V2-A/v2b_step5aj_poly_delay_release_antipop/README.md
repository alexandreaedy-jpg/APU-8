# V2B Step5AJ Poly Delay Release Anti-Pop ROM

Base: Step5AI expressive FX + TRI glide.

Objectif: garder le son valide de Step5AI, corriger la fin de course du release, recuperer le controle LFO delay, et ajouter le mode global/polyphonique `P1/P2/TRI`.

Changements par rapport a Step5AI:

- `Release` rallonge d'environ 50% par rapport a Step5AI.
- `CC34 LFO delay` est a nouveau actif en MIDI.
- Le panel global applique aussi les controles LFO compatibles a `TRI`.
- `CH16` devient aussi un mode note global/polyphonique:
  - allocation sur `P1`, `P2`, puis `TRI`;
  - note-off relache la voix assignee;
  - vol de voix oldest-first si plus de 3 notes sont tenues.
- Ajout d'un duck tres court autour des ecritures pulse high timer `$4003/$4007` pendant le glide pour reduire les pops de phase reset.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5AJPolyDelayReleaseAntiPop`
- firmware: `FW=t65-v2b-step5aj-polydelay`
- short-command target: `midi-v2b14`
