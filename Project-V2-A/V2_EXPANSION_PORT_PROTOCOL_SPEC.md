# V2 Expansion Port - Specification de Protocole

Date de reference : 2026-05-08

> Proposition de protocole concue pour :
> - rester dans les I/O standard
> - etre simple et robuste
> - supporter des controles riches a cadence elevee
> - exploiter le `Pico` comme moteur de transport deterministe

## 1) Modele de communication

- `NES / ROM` = maitre
- `module` = esclave repondant sur les lignes de lecture

### Canal commande
- `$4016` write
- `OUT0-OUT2`

### Canal data
- `$4017` read
- `D0-D4`

### Event optionnel
- `/IRQ`

## 1.1) Intention reelle du proto A/B

Le but de `V2_A / V2_B` n'est pas seulement d'ajouter des lignes.

Le but est d'utiliser le `Pico` pour apporter :
- une file d'evenements propre
- des paquets atomiques
- `ack / ready / sync`
- une meilleure tenue de l'ordre des changements
- une meilleure coexistence entre `notes` et `controles`

La priorite du proto est donc :
- **stabilite et discipline protocolaire**

avant :
- bande passante brute theorique

## 2) Encodage proto 1 retenu

### OUT0-OUT2

Dans le **proto 1**, `OUT[2:0]` ne sert pas a "programmer" directement un registre depuis la NES.

Il sert a **demander un champ** du prochain evenement de controle prepare par le Pico.

`OUT[2:0]` porte donc l'opcode / phase :

- `000` = `IDLE`
- `001` = `READ_STATUS`
- `010` = `READ_REG_ID`
- `011` = `READ_VALUE_LO`
- `100` = `READ_VALUE_HI`
- `101` = `ACK_AND_NEXT`
- `110` = `READ_META`
- `111` = `SYNC / RESERVED`

### D0-D4

- `D0-D3` = nibble
- `D4` = flag `VALID`

Dans ce proto 1 :
- l'opcode selectionne deja le champ (`status`, `reg id`, `value low`, `value high`)
- `D4 = 1` veut dire "donnee valide"
- `D4 = 0` veut dire "pas de donnee utile / pas d'evenement pret"

Un octet est reconstruit en deux lectures :
- `READ_VALUE_LO`
- `READ_VALUE_HI`

## 3) Registres logiques proposes

Premier espace de registres :

- `0x0` = `DUTY`
- `0x1` = `ADSR_A`
- `0x2` = `ADSR_D`
- `0x3` = `ADSR_S`
- `0x4` = `ADSR_R`
- `0x5` = `LFO_RATE`
- `0x6` = `LFO_DEPTH`
- `0x7` = `LFO_SHAPE`

Le module et la ROM ne poussent pas directement des registres APU bruts.
Ils echangent des registres logiques que le moteur applique ensuite proprement.

## 4) Structure d'un evenement proto 1

Le Pico maintient un **evenement courant** ou une petite file d'evenements.

Chaque evenement contient au minimum :
- `reg_id` sur `4 bits`
- `value` sur `8 bits`

Et optionnellement plus tard :
- `target / voice`
- `flags`

### `READ_STATUS`

Je recommande ce mapping simple :

- `D0` = `PENDING`
- `D1` = `OVERFLOW`
- `D2` = reserve
- `D3` = reserve
- `D4` = `VALID`

Interpretation :
- si `PENDING=0`, la ROM n'insiste pas
- si `PENDING=1`, la ROM peut lire `REG_ID`, `VALUE_LO`, `VALUE_HI`

### `READ_REG_ID`

- `D0-D3` = `reg_id`
- `D4` = `VALID`

### `READ_VALUE_LO`

- `D0-D3` = nibble bas de `value`
- `D4` = `VALID`

### `READ_VALUE_HI`

- `D0-D3` = nibble haut de `value`
- `D4` = `VALID`

### `ACK_AND_NEXT`

La ROM pulse cette phase quand elle a bien consomme l'evenement courant.

Effet attendu cote Pico :
- depiler l'evenement courant
- preparer le suivant si disponible

### Regle d'atomicite

Tant qu'un evenement n'a pas ete `ACK` :
- son `REG_ID`
- son `VALUE_LO`
- son `VALUE_HI`

doivent rester figes.

Le Pico ne doit jamais modifier un paquet en cours de lecture.

### `READ_META`

Reserve pour une extension proche :
- `target voice`
- `flags`
- ou `page`

Le proto 1 peut completement l'ignorer au debut.

## 5) Robustesse

### Resync
- reserver `RegID = 0xF` comme token de sync
- ajouter un pattern fixe

### Checksum
- checksum XOR simple sur bloc

### Politique d'erreur
- checksum invalide = bloc ignore
- resync immediat au token suivant

## 6) Cadence

### Mode normal
- polling leger permanent
- cadence cible `120 Hz`

### Mode turbo
- `240 Hz` si les modulations l'exigent

### Option IRQ
- le module peut signaler `data ready`
- utile pour eviter un polling trop aggressif

## 7) Premiere implementation conseillee

Ordre de bring-up logiciel :

1. `READ_STATUS`
2. si `PENDING=1`, alors `READ_REG_ID`
3. `READ_VALUE_LO`
4. `READ_VALUE_HI`
5. `ACK_AND_NEXT`

Commencer par un seul registre :
- `DUTY`

Puis etendre a :
- `ADSR`
- `LFO`

### Transaction type proto 1

Exemple de boucle ROM :

1. ecrire `READ_STATUS` sur `OUT0..OUT2`
2. lire `$4017`
3. si `PENDING=0`, sortir
4. ecrire `READ_REG_ID`
5. lire `$4017`
6. ecrire `READ_VALUE_LO`
7. lire `$4017`
8. ecrire `READ_VALUE_HI`
9. lire `$4017`
10. appliquer localement la nouvelle valeur
11. ecrire `ACK_AND_NEXT`
12. faire une lecture `$4017` ou un petit delai selon le firmware final

### Pourquoi cette forme est retenue

Parce qu'elle colle mieux a la realite du proto :
- `OUT0..OUT2` ne portent que `3 bits`
- le Pico est naturellement source des changements de controles
- la ROM doit surtout **consommer** un evenement, pas "ecrire un registre" a l'aveugle
- la file d'evenements du `Pico` doit absorber la variabilite en amont

## 8) Rappel de doctrine

Le protocole V2 sert les controles riches.
Le groove live critique reste protege par `V1 / D0`.

Voir aussi :
- [V2_EXPANSION_PORT_PLAYBOOK.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_PLAYBOOK.md)
- [V2_EXPANSION_PORT_TEST_PLAN.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_TEST_PLAN.md)
