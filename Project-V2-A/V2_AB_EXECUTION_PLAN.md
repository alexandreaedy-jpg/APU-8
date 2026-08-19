# V2 A/B - Execution Plan

Date de reference : 2026-05-10

## But

Maximiser les capacites de `V2_A / V2_B` avant toute escalation vers `V2_C`.

Le principe directeur est simple :
- exploiter a fond le `Pico`
- garder la nappe actuelle
- pousser le protocole deterministe aussi loin que possible
- ne pas confondre "plus de bande passante brute" avec "meilleur systeme"

## Ce qu'on veut vraiment prouver

`V2_A / V2_B` doivent verifier l'hypothese suivante :

- un `RP2040 Pico`
- plus `OUT0..OUT2`
- plus `/OE2`
- plus `Joypad D0..D4`
- plus une vraie file d'evenements
- plus `ack / ready / sync / paquets atomiques`

...peuvent suffire a obtenir :
- des controles riches utiles
- un systeme plus propre que `V1.7 / V1.8`
- sans degradation du groove notes/triggers

## Lignes actives a exploiter

### Actif proto A/B
- `OUT0`
- `OUT1`
- `OUT2`
- `/OE2`
- `Joypad D0`
- `Joypad D1`
- `Joypad D2`
- `Joypad D3`
- `Joypad D4`

### Debug bonus deja disponible
- `A15`
- `CPU D0`
- `CPU D1`
- `CPU D2`

### Volontairement hors scope pour l'instant
- `/IRQ`
- `CPU D3..D7`
- `R/W`
- `M2`

## Roadmap dev

## Etape 1 - Bring-up electrique

Objectif :
- valider la nappe
- valider `4050`
- valider `245`
- valider les niveaux logiques

Attendus :
- lecture propre de `OUT0..OUT2`
- detection propre de `/OE2`
- emission stable de motifs sur `Joypad D0..D4`
- `245` inactif au boot, actif seulement quand voulu

## Etape 2 - Firmware Pico minimal

Objectif :
- creer un coeur de protocole tres simple mais solide

Premiers blocs firmware :
- lecture de `OUT0..OUT2`
- observation de `/OE2`
- etat protocolaire minimal
- un evenement fixe en memoire
- reponse stable sur `Joypad D0..D4`

Le firmware ne doit pas encore etre "riche".
Il doit etre :
- lisible
- deterministe
- facile a tracer

## Etape 3 - ROM NES minimaliste

Objectif :
- consommer proprement un evenement Pico

Premier cycle ROM :
1. `READ_STATUS`
2. `READ_REG_ID`
3. `READ_VALUE_LO`
4. `READ_VALUE_HI`
5. `ACK_AND_NEXT`

But :
- prouver qu'un paquet complet peut etre lu sans ambiguite

## Etape 4 - Paquets atomiques

Objectif :
- garantir qu'un evenement ne change pas pendant sa lecture

Regles cote Pico :
- l'evenement courant est fige
- il ne change pas tant que la ROM n'a pas fait `ACK`
- le suivant attend dans la file

Cette etape est critique :
- si elle echoue, `V2_A / V2_B` perdent une grande partie de leur interet

## Etape 5 - File d'evenements

Objectif :
- profiter du `Pico` la ou le Nano etait faible

La file doit permettre :
- accumulation de plusieurs changements MIDI
- service propre par la ROM
- conservation de l'ordre
- elimination des faux etats intermediaires

Le vrai gain attendu ici n'est pas juste la vitesse :
- c'est la discipline du transport

## Etape 6 - Priorisation

Objectif :
- garantir que les notes et triggers ne soient pas pollues par les controles

Strategie :
- traiter la voie `notes/triggers` comme prioritaire
- servir les controles comme une file separee
- limiter ou grouper les updates non critiques si besoin

## Etape 7 - Premier controle musical

Commencer par :
- `DUTY`

Puis :
- `ADSR` simple
ou
- `LFO rate/depth`

Pourquoi cet ordre :
- `DUTY` est simple a entendre
- `ADSR` et `LFO` testent la vraie tenue du systeme sous activite

## Etape 8 - Stress mesurable

Objectif :
- sortir du "ca a l'air de marcher"

Mesures qualitatives attendues :
- pas de regression nette sur le groove
- pas de modulation erratique
- pas de blocage du protocole
- pas de perte evidente d'evenements

## Regles d'arret

On ne passe pas a `V2_C` juste par impatience.

On y passe seulement si :
- `V2_A / V2_B` sont correctement mises en oeuvre
- le protocole Pico est propre
- et malgre ca, la limite de capacite reste evidente

## Conclusion

Le vrai chantier `V2_A / V2_B` n'est pas :
- "utiliser un autre port"

Le vrai chantier est :
- construire enfin un transport propre, bufferise, deterministe et atomique

Si ce point est bien execute, `V2_A / V2_B` peuvent deja produire une difference tres nette face a `V1.7 / V1.8`.
