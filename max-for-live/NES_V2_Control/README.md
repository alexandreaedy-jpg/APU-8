# NES V2 Control

Premier panneau de controle Max/Max for Live pour le moteur `separate-channels`.

Cette version fonctionne comme un petit editeur de presets par cible :

- boutons `P1 / P2 / TRI / NOISE / GLOBAL`
- etat memorise separement pour chaque cible
- labels `Ctrl1..4` renommes automatiquement selon la cible

## Fichiers

- `NES_V2_Control.amxd`
- `NES_V2_Control.maxpat`
- `nes_cc_controller.js`

## Usage rapide

1. Glisse [NES_V2_Control.amxd](C:/Users/mto1/Documents/NES_DEV/max-for-live/NES_V2_Control/NES_V2_Control.amxd) sur une piste MIDI dans Live.
2. Si besoin de debug ou d'edition plus tard, ouvre aussi [NES_V2_Control.maxpat](C:/Users/mto1/Documents/NES_DEV/max-for-live/NES_V2_Control/NES_V2_Control.maxpat) dans Max.
3. Route la sortie MIDI de la piste vers ton port virtuel utilise par le bridge NES.
4. Choisis la cible avec les boutons :
   - `P1`
   - `P2`
   - `TRI`
   - `NOISE`
   - `GLOBAL`
5. Change les valeurs ADSR et les `4` controles renommes automatiquement.

## Mapping

ADSR :

- `Attack` -> `CC73`
- `Decay` -> `CC75`
- `Sustain` -> `CC71`
- `Release` -> `CC72`

Controles secondaires affiches selon la cible :

- `P1 / P2`
  - `Ctrl1` -> `CC1` vibrato depth
  - `Ctrl2` -> `CC76` vibrato rate
  - `Ctrl3` -> `CC5` glide
  - `Ctrl4` -> `CC16` duty
- `TRI`
  - `Ctrl1` -> `CC1` vibrato depth
  - `Ctrl2` -> `CC76` vibrato rate
  - `Ctrl3` -> `CC5` glide
  - `Ctrl4` -> `CC24` punch
- `NOISE`
  - `Ctrl1` -> `CC16` timbre
  - `Ctrl2` -> `CC1` mode
- `GLOBAL`
  - `Ctrl1` -> `CC24` arp on/off
  - `Ctrl2` -> `CC26` arp mode
  - `Ctrl3` -> `CC27` arp speed
  - `Ctrl4` -> `CC16` duty global

## Presets

Le patch contient un objet `preset` standard Max :

- clic simple : rappelle un preset
- `shift + clic` : memorise l'etat courant

## Notes

- `refresh` renvoie tous les CC de l'etat courant vers la cible selectionnee
- `panic_all` envoie `CC120` et `CC123` sur `ch12..16`
- c'est une premiere base simple, pas encore un device M4L tres "beau"
