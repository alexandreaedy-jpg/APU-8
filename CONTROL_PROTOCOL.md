# Protocole De Controle NES MIDI v1

Ce document formalise le protocole de controle actuellement utilise en phase 1 entre :

- Ableton / source MIDI
- `bridge.py`
- `midi.lua`
- la ROM NES dans `project-clean/main.c`

L'objectif est de figer une base claire pour la transition vers Arduino Nano + flash cart en phase 2.

## Vue d'ensemble

Flux actuel :

1. le bridge recoit des `note on/off` et des `CC`
2. le bridge maintient un etat global + une liste de 0 a 4 notes actives
3. le bridge serialize cet etat dans `state.bin`
4. `midi.lua` lit `state.bin` a chaque frame et copie 24 octets en RAM NES a partir de `$0200`
5. `main.c` lit ces 24 octets et pilote l'APU

## Taille Et Adresse Du Bus

- taille totale : `24` octets
- fichier intermediaire : `C:\Users\mto1\Documents\NES_DEV\state.bin`
- RAM NES cible : `$0200-$0217`

## Layout Du Bus v1

| Offset | RAM    | Nom              | Taille | Plage    | Description |
|-------:|--------|------------------|-------:|----------|-------------|
| 0      | `$0200`| `seq`            | 1      | `0..255` | compteur d'etat, increment a chaque changement |
| 1      | `$0201`| `mode`           | 1      | `0..2`   | `0=mono`, `1=duo`, `2=poly` |
| 2      | `$0202`| `route`          | 1      | `0..7`   | masque de canaux `bit0=P1`, `bit1=P2`, `bit2=TRI` |
| 3      | `$0203`| `attack`         | 1      | `0..127` | controle ADSR |
| 4      | `$0204`| `decay`          | 1      | `0..127` | controle ADSR |
| 5      | `$0205`| `sustain`        | 1      | `0..127` | controle ADSR |
| 6      | `$0206`| `release`        | 1      | `0..127` | controle ADSR |
| 7      | `$0207`| `vib_depth`      | 1      | `0..127` | profondeur du vibrato |
| 8      | `$0208`| `vib_rate`       | 1      | `0..127` | vitesse du vibrato |
| 9      | `$0209`| `duty`           | 1      | `0..3`   | duty pulse de base |
| 10     | `$020A`| `glide`          | 1      | `0..127` | portamento |
| 11     | `$020B`| `sweep_enable`   | 1      | `0..1`   | reutilise actuellement comme `arp on/off` |
| 12     | `$020C`| `sweep_dir`      | 1      | `0..1`   | reserve / heritage sweep |
| 13     | `$020D`| `sweep_shift`    | 1      | `0..7`   | reutilise actuellement comme `arp mode` |
| 14     | `$020E`| `sweep_period`   | 1      | `0..7`   | reutilise actuellement comme `arp speed` |
| 15     | `$020F`| `active_count`   | 1      | `0..4`   | nombre de slots notes valides |
| 16     | `$0210`| `note0`          | 1      | `0..127` | note active la plus recente |
| 17     | `$0211`| `vel0`           | 1      | `0..127` | velocite associee |
| 18     | `$0212`| `note1`          | 1      | `0..127` | 2e note active |
| 19     | `$0213`| `vel1`           | 1      | `0..127` | velocite associee |
| 20     | `$0214`| `note2`          | 1      | `0..127` | 3e note active |
| 21     | `$0215`| `vel2`           | 1      | `0..127` | velocite associee |
| 22     | `$0216`| `note3`          | 1      | `0..127` | 4e note active |
| 23     | `$0217`| `vel3`           | 1      | `0..127` | velocite associee |

## Regles De Slots Notes

- ordre : `newest-first`
- maximum : `4` notes conservees
- doublons supprimes par numero de note
- `active_count` indique combien de couples `note/vel` sont valides
- les slots restants sont remplis avec `0,0`

Exemple :

- notes jouees : `60`, puis `64`, puis `67`
- slots transmis :
  - `note0=67`, `vel0=...`
  - `note1=64`, `vel1=...`
  - `note2=60`, `vel2=...`
  - `note3=0`, `vel3=0`

## Interpretation Cote ROM

### Mode

- `mono`
  - `P1` joue la note courante
  - `P2` duplique `P1`
  - `TRI` joue l'octave inferieure
  - si arp active et plus d'une note tenue, la note courante est arpgee
- `duo`
  - `P1 = note0`
  - `P2 = note1`
  - `TRI = note0 - 12`
- `poly`
  - `P1 = note0`
  - `P2 = note1`
  - `TRI = note2 - 12`
  - fallback `TRI = note0 - 12` si moins de 3 notes

### Route

Masque binaire :

- `1` = `P1`
- `2` = `P2`
- `4` = `TRI`

Exemples :

- `1` -> seulement `P1`
- `3` -> `P1 + P2`
- `4` -> seulement `TRI`
- `7` -> `P1 + P2 + TRI`

### Enveloppes

- `P1` et `P2` ont maintenant des enveloppes separees
- `TRI` a une voix logique separee avec comportement de release musical, sans vrai volume materiel

### Reaffectation Sweep -> Arpeggiateur

Pour rester compatible avec le bus actuel, les champs sweep ont ete reutilises :

- `sweep_enable` -> `arp on/off`
- `sweep_shift` -> `arp mode`
- `sweep_period` -> `arp speed`
- `sweep_dir` -> reserve

Valeurs arp :

- `arp mode 0` -> `Up`
- `arp mode 1` -> `Down`
- `arp mode 2` -> `Up-Down`

## Mapping MIDI Actuel

Controle par `CC` dans `bridge.py` :

- `CC1` -> vibrato depth
- `CC5` -> glide
- `CC16` -> duty
- `CC20` -> mode
- `CC21` -> route `P1`
- `CC22` -> route `P2`
- `CC23` -> route `TRI`
- `CC24` -> arp on/off
- `CC26` -> arp mode
- `CC27` -> arp speed
- `CC71` -> sustain
- `CC72` -> release
- `CC73` -> attack
- `CC75` -> decay
- `CC76` -> vibrato rate
- `CC120` -> all notes off
- `CC123` -> all notes off

Details specifiques :

- `CC20`
  - `< 43` -> `mono`
  - `< 86` -> `duo`
  - `>= 86` -> `poly`
- `CC21/22/23`
  - `< 64` -> off
  - `>= 64` -> on
- `CC16`
  - mappe sur `0..3`
- `CC26`
  - mappe sur `0..7`, mais seulement `0..2` ont un sens actuellement
- `CC27`
  - mappe sur `0..7`

## Format Serie Recommande Pour Le Nano

Le bridge accepte deja un protocole texte simple sur port serie a `115200 bauds`.

### Messages controle

Format :

```text
C,<KEY>,<VALUE>
```

Exemples :

```text
C,MD,2
C,RT,7
C,A,12
C,R,40
C,GL,64
C,SE,1
C,SS,2
C,SP,5
```

Cles actuellement acceptees :

- `MD` -> mode
- `RT` -> route
- `A` -> attack
- `D` -> decay
- `S` -> sustain
- `R` -> release
- `VD` -> vibrato depth
- `VR` -> vibrato rate
- `DU` -> duty
- `GL` -> glide
- `SE` -> arp on/off
- `SD` -> reserve
- `SS` -> arp mode
- `SP` -> arp speed

### Messages notes

Format :

```text
N,<ON>,<NOTE>,<VEL>
```

Exemples :

```text
N,1,60,100
N,1,64,100
N,0,60,0
```

Regles :

- `ON=1` -> note on
- `ON=0` -> note off
- `NOTE` sur `0..127`
- `VEL` sur `0..127`

## Recommandations Phase 2

Pour l'Arduino Nano, la recommendation actuelle est :

1. garder exactement ce protocole texte serie au debut
2. conserver les memes cles (`MD`, `RT`, `A`, `D`, etc.)
3. laisser `bridge.py` servir d'adaptateur et d'outil de debug pendant les premiers tests hardware
4. ne passer a un protocole binaire Nano -> cart qu'une fois le comportement musical juge stable

Raison :

- plus simple a debugger
- visible dans un terminal serie
- compatible avec l'outillage de phase 1
- evite de changer trop de couches en meme temps

## Points A Figer Avant Hardware Final

Ce qui est deja assez stable :

- taille du bus : `24 octets`
- adresse RAM : `$0200`
- ordre des notes : `newest-first`
- `mode`, `route`, `ADSR`, `glide`, `duty`, `vibrato`
- reutilisation sweep -> arpeggiateur

Ce qui pourra encore evoluer :

- semantique finale de `sweep_dir`
- eventuel retour de la velocite sur le volume
- algorithme final d'allocation de voix
- overlay debug dans la ROM

## Versionnement

Version actuelle recommandee pour la suite :

- `Protocol v1`
- compatible avec le moteur actuel de phase 1
- suffisamment stable pour commencer la preparation Arduino

## Protocol v2 Propose

Cette section decrit une evolution proposee pour supporter un vrai mode multi-channel, sans casser `v1`.

But :

- garder `v1` comme mode instrument global stable
- ajouter un mode de pilotage direct par voix
- reserver une vraie place au canal `NOISE`
- eviter les hacks sur les 4 slots notes de `v1`

### Principe

`v2` ajoute un second layout de bus, plus large, qui se superpose a partir de `$0200` mais sur une taille plus grande.

Recommandation :

- `v1` reste sur `24 octets`
- `v2` passe a `32 octets`
- `midi.lua` devra lire `32` octets au lieu de `24`
- la ROM devra distinguer explicitement `global mode` et `direct channel mode`

### Nouveau Mode

Ajouter un mode de pilotage explicite :

- `mode 0` -> `mono`
- `mode 1` -> `duo`
- `mode 2` -> `poly`
- `mode 3` -> `direct`

Important :

- `mode 3` ne doit pas etre un effet de bord du bridge
- il doit etre un mode officiel du protocole
- il doit etre lu et interprete tel quel par la ROM

### Layout Du Bus v2

Les `24` premiers octets restent identiques a `v1`.

Puis on ajoute :

| Offset | RAM    | Nom            | Taille | Plage    | Description |
|-------:|--------|----------------|-------:|----------|-------------|
| 24     | `$0218`| `p1_note`      | 1      | `0..127` | note directe PULSE1 |
| 25     | `$0219`| `p1_vel`       | 1      | `0..127` | velocite directe PULSE1 |
| 26     | `$021A`| `p2_note`      | 1      | `0..127` | note directe PULSE2 |
| 27     | `$021B`| `p2_vel`       | 1      | `0..127` | velocite directe PULSE2 |
| 28     | `$021C`| `tri_note`     | 1      | `0..127` | note directe TRIANGLE |
| 29     | `$021D`| `tri_vel`      | 1      | `0..127` | velocite logique TRIANGLE |
| 30     | `$021E`| `noise_note`   | 1      | `0..127` | note/percussion pour NOISE |
| 31     | `$021F`| `noise_vel`    | 1      | `0..127` | velocite logique NOISE |

### Regles D'Interpretation

#### Mode global

Si `mode != 3` :

- la ROM ignore `p1_note..noise_vel`
- elle continue a utiliser `active_count + note0..note3`
- comportement identique a `v1`

#### Mode direct

Si `mode == 3` :

- `P1` lit `p1_note/p1_vel`
- `P2` lit `p2_note/p2_vel`
- `TRI` lit `tri_note/tri_vel`
- `NOISE` lit `noise_note/noise_vel`
- `active_count + note0..note3` deviennent facultatifs ou purement informatifs

Recommandation simple :

- en `mode 3`, la ROM ignore totalement `note0..note3`
- toute l'assignation de voix se fait via les champs dedies

### Mapping MIDI Multi-Channel Propose

Mode direct recommande :

- `channel 12` -> `PULSE1`
- `channel 13` -> `PULSE2`
- `channel 14` -> `TRIANGLE`
- `channel 15` -> `NOISE`
- `channel 16` -> `GLOBAL`

Interpretation :

- `ch12..15` alimentent les champs directs dedies
- `ch16` continue de piloter le moteur global `v1`

Recommandation importante :

- ne pas melanger librement `mode 3` et `ch16`
- soit on joue en `global`
- soit on joue en `direct`

Autrement dit :

- `ch16` sert au mode instrument actuel
- `ch12..15` servent au mode direct
- mais pas les deux a la fois sur la meme scene de test

### CC En v2

Pour une premiere iteration, les `CC` peuvent rester globaux :

- `attack`
- `decay`
- `sustain`
- `release`
- `glide`
- `duty`
- `vibrato`
- `route`
- `arp`

Plus tard, si besoin, `v2.1` pourra ajouter des `CC` par voix, mais ce n'est pas necessaire pour debloquer le multi-channel.

### Avantages De v2

- aucune ambiguite entre mode global et mode direct
- aucune surcharge bizarre des slots `note0..note3`
- triangle et noise ont une place officielle
- debug beaucoup plus simple dans l'overlay
- plus facile a reproduire ensuite sur Arduino/Nano

### Recommandation D'Implementation

Ordre conseille :

1. etendre `midi.lua` a `32 octets`
2. ajouter les champs directs dans `bridge.py`
3. ajouter `mode 3 = direct` dans la ROM
4. brancher `P1/P2/TRI`
5. brancher `NOISE`
6. seulement ensuite activer le mapping `ch12..16`

### Conclusion

Le split MIDI par canaux est une bonne idee, mais il doit etre traite comme une vraie `Protocol v2`, pas comme un patch de `v1`.

C'est la solution propre pour obtenir :

- `ch12 -> P1`
- `ch13 -> P2`
- `ch14 -> TRI`
- `ch15 -> NOISE`
- `ch16 -> GLOBAL`

sans destabiliser le moteur actuel.
