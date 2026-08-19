# V2B Step5Z - Typed AUX Transport

## Pourquoi

Le log `STEP2B-Panel_Control_test16.txt` montre `OVF=0` et un `QH` modéré, mais encore des événements `AX=DMC:*` au milieu d'un flux `TRI/NOISE`. Le problème n'est donc pas d'abord la taille de queue: c'est l'ambiguité de type sur la lane `AUX`.

Les versions précédentes protégeaient le DMC avec des valeurs `FD/FE + E0..EF`, mais la ROM avait encore besoin de regarder la forme du payload pour décider si un byte `AUX` était `DMC`. Sous charge, c'est exactement le genre de logique qui peut transformer un événement `TRI` en sample.

## Changement

Step5Z rend le type `AUX` explicite dans le status byte:

- `D3..D4 = 00`: `NOISE` compact, lecture `AUX_LO` seulement
- `D3..D4 = 01`: `TRI` full byte, lecture `AUX_LO + AUX_HI`
- `D3..D4 = 10`: `DMC` full byte, lecture `AUX_LO + AUX_HI`
- `D3..D4 = 11`: `NOISE` full byte, utilise pour controles/meta/value

La ROM ne deduit plus jamais `DMC` a partir de `FD/FE/E*`. Si le status dit `TRI`, l'evenement est `TRI`, meme si sa valeur ressemble a un trigger DMC. Si le status dit `NOISE`, l'evenement reste `NOISE`.

## Invariants

- `P1` et `P2` restent natifs et inchanges.
- `TRI` est toujours full byte sur `AUX`.
- `NOISE` note/gate reste compact pour garder de la bande passante.
- `NOISE` controle passe en full byte type `NOISE`, pour eviter toute collision avec `DMC`.
- `DMC` reste trigger-only sur `CH11`; `DMC pitch` reste desactive.

## Limites Attendues

Cette passe corrige l'erreur d'instrument `TRI -> DMC`. Elle ne rend pas la lane `AUX` infinie: `TRI`, `NOISE`, `DMC` et les controles `NOISE` partagent toujours le meme canal physique. Si la charge est trop forte, le DMC doit etre droppe ou retarde, jamais substitue a une autre voix.

## Validation

Firmware attendu:

- `FW=t55-v2b-step5z-typedaux`

Tests prioritaires:

- `TRI` notes longues et courtes, avec `DMC` active: aucun sample au note-on ou note-off TRI.
- `TRI + NOISE + DMC`: pas de permutation d'instrument.
- `NOISE` controles sous charge: controles possiblement retardes, mais pas transformes en DMC.
- Logs: `AX=DMC:*` doit apparaitre uniquement quand un vrai trigger `CH11` est transporte.

