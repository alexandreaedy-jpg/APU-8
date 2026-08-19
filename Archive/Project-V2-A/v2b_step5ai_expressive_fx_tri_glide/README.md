# V2B Step5AI Expressive FX + TRI Glide ROM

Base: Step5AH stream fast-audio guard, validee stable en full lane.

Objectif: pousser les effets sans casser le groove valide en Step5AH.

Changements par rapport a Step5AH:

- Meme protocole stream stable: `P1/P2/TRI/NOISE/EVENT`.
- Courbe pitch LFO etendue: le bas de course reste subtil, les derniers crans deviennent des profondeurs special-FX.
- `Release` raccourci: la fin de course correspond environ a l'ancien 30%.
- `Decay=0` devient un tick tres court; `Decay=15` reste le mode sustain/held.
- `TRI` gagne le glide ROM-side en jeu legato, avec glide coupe quand l'arp est actif.
- `TRI` peut recevoir le pitch LFO, rate, delay et wave; l'effet ne coute du CPU que s'il est active.
- `P1/P2/TRI` conservent l'ARP cote Pico.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5AIExpressiveFxTriGlide`
- firmware: `FW=t64-v2b-step5ai-fxtri`
- short-command target: `midi-v2b13`
