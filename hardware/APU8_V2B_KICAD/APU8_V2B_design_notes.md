# APU-8 V2B - design notes pour schema KiCad

## 1. Base gelee retenue

Le systeme se compose de 4 blocs principaux :

1. `NES expansion port`
2. `RP2040 Pico`
3. `MCP3008` pour le panel analogique
4. buffers de niveau :
   - `74HC4050` pour `NES 5V -> Pico 3.3V`
   - `74HCT245` pour `Pico 3.3V -> NES 5V`

## 2. Lignes actives minimales

### NES -> Pico via `74HC4050`

- `OUT0` -> `GP2`
- `OUT1` -> `GP3`
- `OUT2` -> `GP4`
- `/OE2` -> `GP5`

### Pico -> NES via `74HCT245`

- `GP10` -> `PORT1-0`
- `GP11` -> `PORT1-1`
- `GP12` -> `PORT1-2`
- `GP13` -> `PORT1-3`
- `GP14` -> `PORT1-4`

### Panel numerique direct sur Pico

- `GP8` = selecteur cible LFO Pitch
- `GP9` = selecteur cible LFO Duty
- `GP15` = selecteur cible LFO Amp
- `GP20` = voice select P1
- `GP21` = voice select P2
- `GP22` = voice select TRI
- `GP26` = voice select NOISE
- `GP27` = voice select GLOBAL
- `GP28` = switch ARP enable

### MCP3008

- `GP16` = MISO
- `GP17` = CS
- `GP18` = SCK
- `GP19` = MOSI

Canaux analogiques :

- `CH0` = Attack
- `CH1` = Volume
- `CH2` = Decay
- `CH3` = Release
- `CH4` = LFO Depth
- `CH5` = LFO Rate
- `CH6` = Duty / Noise timbre
- `CH7` = Arp Time / LFO Waveform

## 3. Lignes optionnelles / debug

### NES -> Pico via `74HC4050`

- `A15` -> `GP6` (optionnel/debug)
- `CPU D0`
- `CPU D1`
- `CPU D2`

Remarque :

- le sketch actuel contient encore le support `A15`
- mais le systeme principal n'en depend pas pour fonctionner musicalement

## 4. Alimentation et bonnes pratiques

- `74HC4050` alimente en `3.3V`
- `74HCT245` alimente en `5V`
- `MCP3008` a alimenter selon le domaine retenu pour les potentiometres et la logique SPI
- masses communes entre NES, Pico, panel et buffers
- decouplage recommande :
  - `100 nF` au plus pres de chaque CI
  - `10 uF` sur le rail `3.3V`
  - `47 uF` a `100 uF` sur le rail `5V`

## 5. Point important a ne pas re-casser

La correction `TRI/DMC` est au niveau logiciel ROM et ne doit pas etre reouverte pendant le travail hardware :

- `update_triangle()` ne doit pas reecrire `$4015`

Ce point n'affecte pas directement le schema, mais il fait partie de l'etat "stable" du projet.
