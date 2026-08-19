# V2A Step 1 - OUT One-Hot Probe

## But

Prouver sans ambiguite quels bits `OUT0..OUT2` passent vraiment jusqu'au Pico.

Ce test ne cherche pas encore a valider :
- `/OE2`
- `A15`

## Ce que fait la ROM

La ROM ecrit lentement dans `$4016` la sequence :
- `001`
- `010`
- `100`
- `000`

Chaque etat est garde longtemps, avec une petite variation audible sur `PULSE1`.

## Ce qu'on attend cote Pico

Sur le moniteur serie :
- `OUT=001`
- puis `OUT=010`
- puis `OUT=100`
- puis `OUT=000`

Si tu ne vois que `001` et `000`, alors :
- `OUT0` passe
- `OUT1` et `OUT2` ne passent pas encore correctement

## Fichiers

- ROM : [main.c](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/v2a_step1_out_onehot_probe/main.c)
- build : [build.ps1](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/v2a_step1_out_onehot_probe/build.ps1)
- sketch Pico : [PicoNesV2A_Step1Probe.ino](/C:/Users/mto1/Documents/NES_DEV/arduino/PicoNesV2A_Step1Probe/PicoNesV2A_Step1Probe.ino)
