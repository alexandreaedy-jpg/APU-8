# V2B Step5AH - Stream Fast Audio Guard

Date: 2026-05-27

Base: Step5AG `midi-v2b11` / `FW=t62-v2b-step5ag-stream5`.

## Diagnostic test24/test26

- Step5AG donne un gros gain: `QH` reste bas (`8..13`) et `OVF=0`.
- Les problemes restants ne ressemblent plus a une saturation de file.
- P1/P2 perdent un peu de groove et LFO/glide deviennent moins stables quand le stream transporte plusieurs lanes + EVENT.
- Cause probable: un paquet stream complet coute plus de lectures bus qu'avant, et chaque lecture utilisait encore le gros `delay_short(1)` cote ROM.
- Les controles peuvent aussi entrer trop souvent sur EVENT pendant un jeu dense, ce qui ajoute du travail ROM et peut donner une sensation de note mutee ou d'effet instable.

## Changements

- Nouveau target short-command: `midi-v2b12`.
- Pico: `arduino\PicoNesV2B_Step5AHStreamFastAudioGuard`.
- ROM: `Project-V2-A\v2b_step5ah_stream_fast_audio_guard`.
- Firmware: `FW=t63-v2b-step5ah-fastguard`.
- ROM: remplace le delai bus grossier `delay_short(1)` par `BUS_SETTLE_NOPS=24`.
- ROM: le service audio/LFO se cadence sur le nombre de lanes traitees par paquet stream, pas seulement sur le nombre de paquets.
- Pico: controles EVENT plus prudents pendant le jeu dense:
  - `PANEL_CONTROL_INJECT_INTERVAL_MS=24`
  - `PANEL_CONTROL_RELEASE_Q=0`
- Le protocole stream Step5AG est conserve: `P1/P2/TRI/NOISE/EVENT`.
- Le fix TRI/DMC est conserve: `update_triangle()` ne touche pas `$4015`.

## A verifier

- `FW=t63-v2b-step5ah-fastguard` dans le log.
- P1/P2 doivent avoir moins de micro-flam/groove drift.
- LFO/glide doivent rester plus reguliers en 4-5 voix.
- `OVF` doit rester a `0`.
- Si la communication bus devient instable, remonter `BUS_SETTLE_NOPS` a `32` ou `40`.
