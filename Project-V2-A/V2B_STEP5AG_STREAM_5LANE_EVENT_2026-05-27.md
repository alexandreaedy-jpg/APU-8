# V2B Step5AG - Stream 5-Lane + Event

Date: 2026-05-27

Base: Step5AF `midi-v2b10` / `FW=t61-v2b-step5af-noisebound`.

## Pourquoi

Step5AF stabilise le full-lane en bornant `NOISE`, mais l'ancien protocole garde `TRI/NOISE/DMC` dans un `AUX` multiplexe. Sous charge, `NOISE` peut encore accumuler du retard parce qu'il partage ses slots avec `TRI` et les evenements.

Le materiel a 5 lignes data `D0..D4`, mais seulement 3 bits d'opcode, donc 8 commandes de lecture. Une vraie architecture 5 lanes avec `LO/HI` separe pour chaque voix demanderait trop d'opcodes. Step5AG contourne proprement cette limite avec un protocole stream.

## Protocole

- `STATUS` utilise les 5 bits comme lanes logiques:
  - `D0 = P1 pending`
  - `D1 = P2 pending`
  - `D2 = TRI pending`
  - `D3 = NOISE pending`
  - `D4 = EVENT pending`
- La ROM lit ensuite une seule commande `READ_DATA` autant de fois que necessaire.
- Ordre du flux:
  - `P1`: full byte `LO/HI`
  - `P2`: full byte `LO/HI`
  - `TRI`: full byte `LO/HI`
  - `NOISE`: short byte 5 bits `note4 + gate`
  - `EVENT`: header voix, puis full byte `LO/HI`
- `EVENT` transporte DMC + controles CC/panel coalesces.

## Effet attendu

- `TRI` ne partage plus `AUX` avec `NOISE`.
- `NOISE` ne partage plus ses slots avec `TRI`; il ne partage indirectement que la cadence globale du service loop.
- DMC et controles restent sur une lane moins critique.
- Le correctif TRI/DMC reste conserve: `update_triangle()` ne touche pas `$4015`.
- Glide P2 conserve depuis Step5AF.

## A verifier au test

- Le log doit annoncer `FW=t62-v2b-step5ag-stream5`.
- En full-lane, `T` et `N` doivent rester beaucoup plus bas qu'avant.
- `OVF` doit rester a `0`.
- `ND` ne devrait monter que si la partie NOISE seule depasse sa propre lane.
- P2 glide doit toujours etre audible.
