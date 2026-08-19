# V2B Step5AI - Expressive FX + TRI Glide

Date: 2026-05-27

Base: Step5AH `midi-v2b12` / `FW=t63-v2b-step5ah-fastguard`, validee stable par test audio full lane avec ARP + LFO.

## Objectif

Garder la stabilite Step5AH, puis agrandir la palette expressive:

- LFO depth subtil en bas de course, tres exagere en fin de course.
- Release beaucoup plus court: l'ancien ressenti vers 30% devient la nouvelle fin de course.
- Decay minimum transforme en tick tres court.
- Glide sur `TRI`.
- `TRI` accepte le pitch LFO, sans toucher au correctif DMC.

## Changements ROM

- `ctrl_to_vibrato_depth()` utilise une table non lineaire:
  - bas de course proche de Step5AH;
  - derniers crans `13..15` reserves aux effets speciaux.
- `env_release_duration()` passe de `4*release/level` a `(release+4)/level`.
- `Decay=0` garde un seul tick fort puis coupe la voix.
- `Decay=15` reste le seul mode sustain.
- Ajout d'un timer glide specifique `TRI` pour conserver le demi-timer triangle.
- `TRI` glide en legato si l'arp n'est pas actif.
- `TRI` pitch LFO reactive seulement quand profondeur + rate sont actifs.

## Changements Pico

- Nouveau firmware: `FW=t64-v2b-step5ai-fxtri`.
- `TRI` accepte maintenant `LFO depth/rate/delay/wave` + `mode flags`.
- `P1`, `P2` et `TRI` restent compatibles ARP.
- Le scheduler controles Step5AH est conserve (`Q=0`, intervalle `24 ms`).

## A verifier

- Log: `FW=t64-v2b-step5ai-fxtri`.
- Full lane doit garder `OVF=0`.
- Tester d'abord sans TRI LFO pour confirmer que Step5AH reste intacte.
- Tester ensuite `TRI` glide en legato hors ARP.
- Tester `TRI` arp: le glide doit se couper pendant l'ARP.
- Tester LFO depth `0..10` musical, puis `13..15` comme zone special-FX.
- Tester release: le maximum doit ressembler a l'ancien 30%.
- Tester decay minimum: le son doit devenir un tick tres court.
