# V2B Step5K Panel Safe

Firmware: `t40-v2b-step5k-panelsafe`

## But

Stabiliser le panneau physique sans toucher au transport note/ARP valide.

## Changements

- Le selecteur de voix est accepte uniquement si une seule ligne est active.
- Le selecteur de cible LFO est accepte uniquement si une seule ligne est active.
- Les heartbeats ajoutent `VM=` et `LM=` pour voir les masques bruts des selecteurs.
- `Attack` panel a maintenant une zone basse plus large pour eviter d'effacer la note trop vite.
- `Release` panel redescend explicitement a zero en bas de course.
- `NOISE` est volontairement strict:
  - accepte `volume`
  - accepte `release`
  - accepte `duty/timbre`
  - ignore `attack`, `decay`, `LFO`, `arp` et autres controles parasites

## Lecture log

- `PV=` montre les 8 entrees MCP3008 dans l'ordre `V A D R LD LR DU AR`.
- `PA=` montre le dernier controle analogique applique.
- Si le slider Volume ne fait jamais bouger le premier nibble de `PV`, le probleme est probablement le cablage ou le mapping MCP3008, pas le transport.
- `VM=` doit avoir une seule valeur active stable:
  - `1` P1
  - `2` P2
  - `4` TRI
  - `8` NOISE
  - `10` GLOBAL
- `LM=` doit avoir une seule valeur active stable:
  - `1` pitch
  - `2` duty
  - `4` amp

## Compromis

Cette revision privilegie la fiabilite live. Les controles Noise riches pourront revenir plus tard, mais uniquement si leur transport est separe ou mieux cadence.
