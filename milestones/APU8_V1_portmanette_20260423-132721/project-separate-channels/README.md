# Separate Channels Sandbox

Projet experimental pour tester une ROM NES dediee au mode multi-canaux, sans toucher a la base stable de `project-clean`.

## Objectif

Mapper directement des voix logiques vers l'APU :

- `P1`
- `P2`
- `TRI`
- `NOISE`

Sans logique `mono/duo/poly` ni voice allocator global.

## Bus v2 (58 octets)

RAM cible : `$0700-$0739`

- `$0700` `seq`
- `$0701` `attack`
- `$0702` `decay`
- `$0703` `sustain`
- `$0704` `release`
- `$0705` `vib_depth` reserve
- `$0706` `vib_rate` reserve
- `$0707` `duty`
- `$0708` `glide`
- `$0709` reserve
- `$070A` `p1_note`
- `$070B` `p1_vel`
- `$070C` `p1_gate`
- `$070D` `p2_note`
- `$070E` `p2_vel`
- `$070F` `p2_gate`
- `$0710` `tri_note`
- `$0711` `tri_vel`
- `$0712` `tri_gate`
- `$0713` `noi_note`
- `$0714` `noi_vel`
- `$0715` `noi_gate`
- `$0716` `p1_duty`
- `$0717` `p1_glide`
- `$0718` `p1_vib_depth`
- `$0719` `p1_vib_rate`
- `$071A` `p2_duty`
- `$071B` `p2_glide`
- `$071C` `p2_vib_depth`
- `$071D` `p2_vib_rate`
- `$071E` `tri_glide`
- `$071F` `tri_vib_depth`
- `$0720` `tri_vib_rate`
- `$0721` `p1_attack`
- `$0722` `p1_decay`
- `$0723` `p1_sustain`
- `$0724` `p1_release`
- `$0725` `p2_attack`
- `$0726` `p2_decay`
- `$0727` `p2_sustain`
- `$0728` `p2_release`
- `$0729` `tri_attack` reserve
- `$072A` `tri_decay` reserve
- `$072B` `tri_sustain` reserve
- `$072C` `tri_release`
- `$072D` `noi_attack`
- `$072E` `noi_decay`
- `$072F` `noi_sustain`
- `$0730` `noi_release`
- `$0731` `noi_timbre`
- `$0732` `noi_mode`
- `$0733` `tri_punch`
- `$0734` `p1_trigger`
- `$0735` `p2_trigger`
- `$0736` `tri_trigger`
- `$0737` `noi_trigger`
- `$0738-$0739` reserves

## Build

```powershell
cd C:\Users\mto1\Documents\NES_DEV\project-separate-channels
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

## Bridge V2

Bridge Python :

```powershell
cd C:\Users\mto1\Documents\NES_DEV
py -3.12 bridge_separate_channels.py
```

Le bridge V2 accepte maintenant aussi une entree serie Nano optionnelle en plus du MIDI.

Protocole serie accepte :

- `CC,<CHANNEL>,<CC>,<VALUE>`
- `NOTE,<CHANNEL>,<ON>,<NOTE>,<VEL>`
- `PANIC`

Exemples :

- `CC,12,73,80`
- `CC,16,24,127`
- `PANIC`

Le port serie est detecte automatiquement via `SERIAL_PORT_HINT = "Arduino"` ou peut etre force dans [bridge_separate_channels.py](C:/Users/mto1/Documents/NES_DEV/bridge_separate_channels.py).

## Nano Controller

Sketch dedie :

- [NanoVoiceController.ino](C:/Users/mto1/Documents/NES_DEV/arduino/NanoVoiceController/NanoVoiceController.ino)

Ce sketch est prevu pour un vrai module de controle :

- selecteur rotatif 5 positions
  - `P1`
  - `P2`
  - `TRI`
  - `NOISE`
  - `GLOBAL`
- `4` sliders ADSR
- `4` controles secondaires
- `1` bouton panic

Le selecteur ne mute aucune voix :

- il change seulement la destination des sliders/potards
- les autres voix continuent de jouer normalement

## V1 hardware prioritaire

Pour la premiere version hardware, le plus important est :

- entree `MIDI DIN`
- reception des `notes on/off`
- reception des `MIDI CC`

Sans controles directs locaux.

Le sketch prioritaire pour cette etape est donc :

- [NanoMidiBridge.ino](C:/Users/mto1/Documents/NES_DEV/arduino/NanoMidiBridge/NanoMidiBridge.ino)

Il envoie maintenant vers le bridge V2 :

- `NOTE,<CHANNEL>,<ON>,<NOTE>,<VEL>`
- `CC,<CHANNEL>,<CC>,<VALUE>`
- `PANIC`

Donc il est adapte au moteur separate-channels avec les canaux MIDI :

- `ch12 -> P1`
- `ch13 -> P2`
- `ch14 -> TRI`
- `ch15 -> NOISE`
- `ch16 -> GLOBAL`

Mapping des controles secondaires :

- `P1/P2`
  - `C1` glide
  - `C2` duty
  - `C3` vibrato depth
  - `C4` vibrato rate
- `TRI`
  - `C1` glide
  - `C2` punch on/off (`CC28`)
  - `C3` vibrato depth
  - `C4` vibrato rate
- `NOISE`
  - `C1` timbre
  - `C2` mode
- `GLOBAL`
  - `C1` glide global
  - `C2` duty global
  - `C3` vibrato depth global
  - `C4` vibrato rate global

Commutateur LFO 3 positions :

- `Pitch`
- `Duty`
- `Amp`

Le switch envoie `CC74` sur la cible courante :

- `0` = `Pitch`
- `64` = `Duty`
- `127` = `Amp`

Dans la ROM actuelle :

- `P1/P2` supportent `Pitch`, `Duty` et `Amp`
- `TRI` retombe sur `Pitch`

## Notes

- ce projet est un bac a sable
- il n'utilise pas le bridge v1 actuel
- il faudra un `state` et un script Lua dedies pour le tester dans Mesen
- FamiTone est desactive dans ce sandbox pour ne pas ecraser l'APU
- `duty`, `glide`, `vibrato depth/rate` sont maintenant par-voix sur `P1`, `P2` et `TRI`
- `ADSR` est maintenant par-voix sur `P1`, `P2` et `NOISE`; `TRI` utilise surtout sa `release` propre
- `TRI` peut maintenant avoir un petit `punch` d'attaque, activable via `CC28` sur `ch14`
- `NOISE` a maintenant :
  - `timbre` direct sur 16 periodes
  - `mode` court/metal via le bit de mode du registre `$400E`
  - comportement drum voice selon la note :
    - notes tres basses : kick
    - notes basses : toms
    - autour de `C3` : kick/tom plus rond
    - notes moyennes : snare plus sec
    - notes medium-hautes : closed hat
    - notes hautes : open hat
    - notes tres hautes : cymbal
- banque DMC "ROM-ready" active dans le sandbox :
  - `kick` DMC pour les notes `36-37`
  - `snare` DMC pour les notes `38-40`
  - `rimshot` DMC pour la note `41`
  - `voice fx` DMC pour la note `46`
  - le reste repasse par le moteur `NOISE` pour toms / hats / cymbales
  - banque compacte `kick / snare / rim / voice` placee en ROM dans le segment `SAMPLES`
  - version actuelle orientee "raw": kick/snare/voice utilisent les donnees DMC d'origine avec une zone ROM DMC elargie
  - source de generation : [tools/make_dmc_rom_ready_kit.py](C:/Users/mto1/Documents/NES_DEV/project-separate-channels/tools/make_dmc_rom_ready_kit.py)
- `CH16` peut maintenant aussi servir de mode instrument :
  - `CC24` arp on/off
  - `CC25` arp sync MIDI clock on/off
  - `CC26` arp mode (`up/down/up-down`)
  - `CC27` arp speed ou subdivision si sync active
- `P1`, `P2` et `TRI` ont maintenant leur arp local auto :
  - il s'active automatiquement quand plusieurs notes sont tenues sur la meme voix
  - `CC25` arp sync MIDI clock on/off
  - `CC26` arp mode
  - `CC27` arp speed ou subdivision si sync active
- l'arp parcourt maintenant les notes par hauteur (`pitch order`) au lieu de l'ordre d'arrivee

## Reception future via port manette NES

Le sandbox contient maintenant une routine de reception directe depuis le port manette NES.

Configuration dans [main.c](C:/Users/mto1/Documents/NES_DEV/project-separate-channels/main.c) :

- `CONTROLLER_RX_ENABLED`
  - `0` = desactive
  - `1` = lit un paquet sur le port manette a chaque frame
- `CONTROLLER_RX_PORT`
  - `1` = lecture sur `$4016`
  - `2` = lecture sur `$4017`
- `CONTROLLER_RX_INVERT`
  - inverse le bit si le montage final l'exige

Le protocole vise un paquet fixe de `58 octets`, en `LSB-first`, directement copie vers `$0700-$0739`.

Pour l'instant, cette voie est desactivee par defaut afin de ne pas casser le pipeline actuel `state file -> Lua -> RAM`.
