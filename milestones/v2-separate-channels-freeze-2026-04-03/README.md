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
- `$0733-$0739` reserves

## Build

```powershell
cd C:\Users\mto1\Documents\NES_DEV\project-separate-channels
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

## Notes

- ce projet est un bac a sable
- il n'utilise pas le bridge v1 actuel
- il faudra un `state` et un script Lua dedies pour le tester dans Mesen
- FamiTone est desactive dans ce sandbox pour ne pas ecraser l'APU
- `duty`, `glide`, `vibrato depth/rate` sont maintenant par-voix sur `P1`, `P2` et `TRI`
- `ADSR` est maintenant par-voix sur `P1`, `P2` et `NOISE`; `TRI` utilise surtout sa `release` propre
- `NOISE` a maintenant :
  - `timbre` direct sur 16 periodes
  - `mode` court/metal via le bit de mode du registre `$400E`
  - comportement drum voice selon la note :
    - notes basses : kick/tom bruites
    - notes moyennes : snare/clap
    - notes hautes : hats/cymbales
- `CH16` peut maintenant aussi servir de mode instrument :
  - `CC24` arp on/off
  - `CC25` arp latch on/off
  - `CC26` arp mode (`up/down/up-down`)
  - `CC27` arp speed
  - `CC28` arp octave spread (`0/1/2`)
