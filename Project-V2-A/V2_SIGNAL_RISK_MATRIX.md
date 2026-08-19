# V2 Signal Risk Matrix

Date de reference : 2026-05-10

## But

Classer les signaux V2 selon :
- leur **interet technique**
- leur **niveau d'invasivite**
- leur **risque materiel**

Objectif :
- avancer prudemment
- ne pas toucher au CPU trop tot
- savoir quelles lignes valent vraiment une modification plus invasive

## Niveaux

### Niveau A - safe sans modif invasive

Signaux deja exposes proprement via l'expansion port ou deja soudes sur la nappe actuelle.

#### Signaux
- `OUT0`
- `OUT1`
- `OUT2`
- `PORT1-0..4`
- `/OE2`
- `+5V`
- `GND`
- `A15`
- `CPU D0`
- `CPU D1`
- `CPU D2`

#### Interet
- bring-up rapide
- proto 1 V2
- separation notes / controles
- premiere couche de diagnostic

#### Risque
- faible si buffers et masses sont corrects
- pas de chirurgie proche CPU

#### Verdict
- **a faire en premier**

## Niveau B - modif interne legere / extension logique

Signaux encore raisonnables si on accepte un peu plus de travail, mais sans attaquer directement les points les plus sensibles.

#### Signaux
- `CPU D3`
- `CPU D4`
- `CPU D5`
- `CPU D6`
- `CPU D7`
- `/IRQ`

#### Interet
- meilleur sniff bus
- diagnostic plus riche
- preparation a une V2 plus puissante

#### Risque
- moyen
- plus de soudures
- plus de complexite de nappe
- plus de chances de faux contact ou erreur de pinout

#### Verdict
- **a envisager seulement si le proto 1 apporte deja quelque chose**

## Niveau C - modif invasive / proche CPU

Signaux qui demandent de reprendre directement des points delicats, souvent proches du CPU, et qu'il faut reserver a une vraie escalation d'architecture.

#### Signaux
- `R/W`
- `M2`
- `A13`
- `A14`

#### Interet
- vraie V2 bus-aware
- savoir quand le bus est valide
- distinguer lecture / ecriture
- decodage d'espace adresse beaucoup plus propre

#### Risque
- plus invasif
- plus delicat mecaniquement
- plus proche des zones ou une erreur peut endommager la console ou la rendre instable

#### Verdict
- **a reporter tant qu'on n'a pas une preuve que le gain justifie la chirurgie**

## Lecture strategique

### Ce qu'on fait maintenant
- rester en `Niveau A`
- terminer le bring-up `Pico + 4050 + 245`
- mesurer ce que le proto 1 apporte vraiment

### Ce qu'on n'exclut pas
- passer en `Niveau B` si la nappe actuelle montre deja une vraie valeur

### Ce qu'on refuse pour l'instant
- sauter directement en `Niveau C`
- repiquer `R/W` et `M2` sans preuve claire que la V2 safe plafonne vraiment

## Conclusion nette

Si l'objectif prioritaire est :
- **ne pas endommager la NES**
- **eviter les modifs invasives**

alors la bonne sequence est :

1. `Niveau A`
2. eventuellement `Niveau B`
3. `Niveau C` seulement si le projet le justifie vraiment

Autrement dit :
- `R/W` et `M2` ne sont **pas abandonnes**
- mais ils sont **volontairement repousses**
