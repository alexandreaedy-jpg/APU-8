# V2B Step5Q - LFO panel exclusif

Revision Pico: `t46-v2b-step5q-lfoexclusive`

Objectif: stabiliser le selecteur physique de destination LFO sans toucher au transport note/arp valide.

## Mapping confirme

- Position 1: Pitch, Pico pin 11 / `GP8`
- Position 2: Duty, Pico pin 12 / `GP9`
- Position 3: Amp, Pico pin 20 / `GP15`

Les entrees restent en `INPUT_PULLUP`, donc le selecteur est lu en actif bas.

## Correction

- Le potentiometre `LFO Depth` est maintenant exclusif.
- Quand `Pitch` est choisi, les profondeurs `Duty` et `Amp` sont remises a zero.
- Quand `Duty` est choisi, les profondeurs `Pitch` et `Amp` sont remises a zero.
- Quand `Amp` est choisi, les profondeurs `Pitch` et `Duty` sont remises a zero.
- Changer la position du selecteur applique immediatement la profondeur courante, sans devoir rebouger le potentiometre.
- La forme d'onde LFO par defaut est maintenant `sine` cote Pico et cote ROM.

## Raison

Avant cette revision, les destinations LFO etaient cumulatives. Une profondeur envoyee en `Duty` pouvait rester active apres un passage en `Amp` ou `Pitch`, ce qui donnait un comportement entendu comme un melange des trois modulations.
