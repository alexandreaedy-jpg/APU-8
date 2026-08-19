# V2A Step 1 - OUT Probe

## But

Valider en tout premier :
- `NES -> Pico`
- `OUT0..OUT2`
- `/OE2`
- `A15`

sans encore activer le vrai protocole de controle.

## Ce que fait ce test

### Cote ROM NES

La ROM :
- ecrit en boucle les valeurs `0..7` dans `$4016`
- ce qui fait vivre `OUT0..OUT2`
- effectue deux lectures `$4017` a chaque pas
- ce qui fait pulser `/OE2`
- fait varier legerement un son `PULSE1` pour confirmer qu'elle tourne

### Cote Pico

Le sketch :
- garde le `74HCT245` desactive par defaut
- observe `OUT0..OUT2`, `/OE2`, `A15`
- imprime les changements en `Serial USB`

## Fichiers

- ROM : [main.c](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/v2a_step1_out_probe/main.c)
- build : [build.ps1](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/v2a_step1_out_probe/build.ps1)
- sketch Pico : [PicoNesV2A_Step1Probe.ino](/C:/Users/mto1/Documents/NES_DEV/arduino/PicoNesV2A_Step1Probe/PicoNesV2A_Step1Probe.ino)

## Resultat attendu

Sur le moniteur serie du Pico :
- changements `OUT=000` a `OUT=111`
- compteur de pulses `/OE2`
- aucun comportement parasite

Le `245` doit rester safe :
- `pin 19 /OE` au haut par defaut
- aucune ligne `Joypad D0..D4` effectivement drivee tant qu'on n'attaque pas l'etape suivante
