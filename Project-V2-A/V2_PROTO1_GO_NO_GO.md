# V2 Proto 1 - Go / No-Go Criteria

Date de reference : 2026-05-10

## But

Traiter `V2_A / V2_B` comme une vraie piste serieuse, sans pour autant s'y enfermer.

Le proto 1 doit etre traite comme une experience empirique disciplinee :
- on valide brique par brique
- on mesure un gain reel
- on exploite a fond les capacites du `Pico`
- sinon on pivote vite

## Regle de base

Le proto 1 V2 n'a **pas besoin** de battre `V1.8` sur tous les plans.

Il doit au minimum demontrer :
- qu'il ne casse pas le groove de `V1.8`
- qu'il apporte une voie de controles plus propre
- qu'il ouvre une trajectoire credible vers une V2 plus forte
- qu'il justifie le passage `Nano -> Pico` par une vraie difference de discipline protocolaire

## Brique 1 - Bring-up electrique

### Succes
- `OUT0..OUT2` lus proprement par le Pico
- `/OE2` detecte proprement
- `Joypad D0..D4` lus proprement par la NES
- aucun CI ne chauffe
- pas de comportement parasite evident
- `245` reste safe au boot

### Echec
- niveaux instables
- faux patterns
- comportement aleatoire
- `245` difficile a tenir safe

### Decision
- si echec ici, **stop immediat** et re-evaluation hardware

## Brique 2 - Premier aller-retour de protocole

### Succes
- la ROM peut demander `STATUS`
- le Pico renvoie un evenement simple
- la ROM lit `REG_ID + VALUE`
- `ACK_AND_NEXT` fonctionne
- l'evenement reste stable tant qu'il n'est pas `ACK`
- pas de faux changement entre deux lectures d'un meme paquet

### Echec
- handshake fragile
- pertes d'evenements
- besoin de hacks timing lourds des le debut
- paquet non atomique
- etat Pico difficile a raisonner

### Decision
- si le handshake de base est deja fragile, **gros signal d'alerte**

## Brique 3 - Premier controle musical utile

Commencer par :
- `DUTY`

### Succes
- variation audible correcte
- effet reproductible
- aucune degradation nette du groove notes/triggers
- le Pico peut accumuler puis servir proprement plusieurs changements rapides

### Echec
- variation trop erratique
- impact audible sur les notes
- comportement peu fiable d'une session a l'autre
- ordre des evenements mal tenu

### Decision
- si meme `DUTY` degrade le groove, **stop probable**

## Brique 4 - Controle un peu plus exigeant

Ajouter :
- `ADSR` simple
ou
- `LFO rate/depth`

### Succes
- la modulation ajoute une vraie valeur musicale
- le moteur reste stable
- la cadence `120 Hz` est deja convaincante
- la priorisation `notes / triggers` vs `controles` reste saine

### Echec
- stepping genant
- surcharge evidente
- le benefice musical ne justifie pas la complexite
- la file d'evenements ne suffit pas a lisser la charge

### Decision
- si le gain musical reste faible, **le proto V2.1 ne vaut sans doute pas plus d'effort**

## Critere GO

Le proto V2.1 merite de continuer si :

1. le bring-up est propre
2. le protocole minimal est stable
3. au moins un controle utile (`DUTY` ou `ADSR`) fonctionne sans polluer le groove
4. la trajectoire vers des controles plus riches parait credible
5. le `Pico` apporte deja une sensation de systeme plus propre que `V1.7 / V1.8`

## Critere NO-GO

On arrete ou on pivote vite si :

1. il faut des hacks timing lourds trop tot
2. le groove se degrade des les premieres briques
3. le gain musical reste marginal
4. la complexite grimpe plus vite que la valeur obtenue
5. le `Pico` ne reussit pas a transformer le protocole en vrai avantage pratique

## Lecture strategique

### Si GO
- continuer en mode empirique
- ajouter une brique a la fois
- ne pas toucher `R/W` / `M2` tant que ce proto 1 prouve sa valeur
- pousser `V2_A`, puis `V2_B`, avant toute tentation `V2_C`

### Si NO-GO
- ne pas insister
- conserver la nappe comme fondation
- pivoter vers une V2 plus franchement bus-aware

## Verdict net

Le proto 1 V2 vaut le coup :
- **s'il montre un gain concret tres tot**
- **et si ce gain vient clairement du `Pico + protocole`, pas seulement du cablage**

Sinon :
- on s'en sert comme apprentissage
- et on passe a une architecture plus ambitieuse sans culpabiliser
