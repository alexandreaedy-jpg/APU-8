# V2 Level C - Bus-Aware Architecture

Date de reference : 2026-05-10

## But

Definir la **vraie V2 bus-aware** comme cible finale :
- plus de bande passante utile
- moins de dependance aux lignes type "joypad"
- meilleure integration avec le CPU NES

Cette doc ne dit pas "on le fait maintenant".
Elle fixe **la cible d'architecture** pour savoir ou mene V2.

## Idee centrale

Le Niveau C ne doit plus etre pense comme :
- un port manette ameliore

Mais comme :
- un **peripherique memory-mappe** ou quasi memory-mappe
- aware du bus CPU
- capable de capter des ecritures de la ROM dans une petite zone dediee

## Deux sous-niveaux utiles

## C1 - Bus-aware write-only (recommande comme premiere vraie cible)

### Philosophie

La ROM **ecrit** dans une petite mailbox.
Le module/Pico **observe**, decode et consomme ces ecritures.

Dans cette version :
- on ne cherche pas encore a faire lire au CPU des octets depuis `CPU D0..D7`
- on ne drive pas le bus data CPU
- on garde un chemin de retour simple via `/IRQ` ou, au pire, un canal secondaire

### Avantage

Tres gros gain de puissance par rapport au proto `OUT + Joypad` :
- `8 bits` de data reels par ecriture
- plus besoin de nibbles sur `Joypad D0..D4`
- architecture beaucoup plus proche d'un vrai bus de registres

### Lignes requises

- `CPU D0..D7`
- `A15`
- idealement `A14`
- idealement `A13`
- `R/W`
- `M2`

Optionnel :
- `/IRQ`

### Rôle des lignes Joypad D0..D4

Dans `C1`, elles ne sont **plus la voie principale**.

On peut les garder seulement comme :
- fallback
- bring-up
- sideband de debug

Mais l'architecture cible ne depend plus d'elles.

### Mode de fonctionnement

La ROM choisit une petite fenetre d'adresses.

Exemple conceptuel :
- `REG_CMD`
- `REG_TARGET`
- `REG_VALUE_LO`
- `REG_VALUE_HI`

La ROM ecrit dedans.
Le module capte :
- l'adresse
- la direction (`R/W`)
- le moment valide (`M2`)
- le byte ecrit (`CPU D0..D7`)

Puis le Pico traduit cela en evenement de controle.

## C2 - Bus-aware complet avec readback

### Philosophie

En plus des ecritures, le CPU peut **lire** un statut / version / ack / buffer level
directement sur le bus data CPU.

### Lignes requises

Les memes que `C1`, mais avec en plus :
- une vraie logique de drive conditionnel du bus CPU

### Risque

Beaucoup plus eleve :
- risque de contention
- timing plus delicat
- validation materielle plus lourde

### Verdict

`C2` est la vraie version "haut de gamme", mais elle ne doit venir qu'apres un `C1` reussi.

## Utilisation de ton stock d'IC

## IC vraiment utiles pour Level C

### `74HCT138`

Role :
- decodeur d'adresse simple

Usage probable :
- decoder une petite fenetre a partir de `A13/A14/A15`

### `74HCT14`

Role :
- nettoyage / inversion / remise en forme

Usage probable :
- qualifier ou nettoyer `M2`
- generer des strobes propres

### `74HCT573`

Role :
- latch 8 bits

Usage probable :
- figer un byte de `CPU D0..D7` lors d'une ecriture valide
- eviter de demander au Pico d'echantillonner le bus "a chaud"

### `74HCT245`

Role :
- bus transceiver 8 bits

Usage probable :
- **pas obligatoire en C1**
- **utile seulement en C2** si on veut que le module puisse repondre directement sur `CPU D0..D7`

### `74HC4050`

Role :
- protection / adaptation `5V -> 3.3V`

Limite :
- un seul boitier ne suffit pas pour tout un bus-aware complet si on route tout en direct au Pico

Conclusion pratique :
- en Niveau C, le mieux est de laisser de la logique `5V` preparer/latcher/decoder
- puis de presenter au Pico un sous-ensemble plus calme et plus propre

## Architecture conseillee avec ton stock

## C1 raisonnable

Je viserais :

1. `A13/A14/A15` -> `74HCT138`
2. `R/W` + `M2` nettoyes/qualifies via `74HCT14`
3. strobe valide -> `74HCT573`
4. `CPU D0..D7` latchees dans le `573`
5. le Pico ne lit pas le bus brut directement, il lit le resultat latche / des flags / une interface simplifiee

Autrement dit :
- la glue logic 5V fait la partie rapide
- le Pico fait la partie intelligente

## Pourquoi je prefere ca

Parce que le Pico ne devrait pas etre oblige de :
- echantillonner le bus CPU nu
- courir apres chaque transition
- reconstruire tout seul un timing trop serre

La logique externe joue ici le role de "front-end de capture".

## Plan dev Niveau C

## Etape C1.1 - choisir la mailbox

Definir une zone d'adresses qui :
- ne casse pas le projet courant
- peut servir de `REG_CMD / REG_VALUE`

## Etape C1.2 - prouver la capture d'ecriture

Objectif :
- une ecriture ROM volontaire
- une capture fiable du byte dans le `573`

Sans encore appliquer quoi que ce soit musicalement.

## Etape C1.3 - premier registre utile

Commencer par :
- `DUTY`

Puis :
- `ADSR`
- `LFO`

## Etape C1.4 - retour vers la ROM

Avant de faire du vrai readback bus CPU, tester :
- `/IRQ`
ou
- un statut minimal secondaire

## Position de Joypad D0..D4 a ce stade

### En proto 1 V2.1
- voie active principale

### En Niveau C1
- fallback / compat / sideband

### En Niveau C2
- potentiellement plus necessaires

## Verdict net

Si on veut une V2 finale vraiment plus puissante :
- le bon objectif n'est pas "mieux utiliser Joypad D0..D4"
- le bon objectif est **C1 bus-aware write-only**, puis eventuellement `C2`

Donc :
- **non**, le Niveau C final n'a pas vocation a dependre de `Joypad D0..D4`
- **oui**, on peut les garder comme roue de secours
- **oui**, `R/W` et `M2` deviennent alors tres probablement des lignes importantes
