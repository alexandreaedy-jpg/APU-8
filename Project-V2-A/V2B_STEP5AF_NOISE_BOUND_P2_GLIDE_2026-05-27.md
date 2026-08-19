# V2B Step5AF - Noise Bound + P2 Glide

Date: 2026-05-27

Base: Step5AE `midi-v2b9` / `FW=t60-v2b-step5ae-hardrt`.

## Diagnostic test23

- Le log `STEP2B-Panel_Control_test23.txt` montre une amelioration versus Step5AC, mais l'instabilite full-lane revient quand la file `NOISE` grimpe fortement.
- Point critique observe: `N` monte autour de 48 evenements, `QH` depasse 50, puis `OVF=1`.
- `MX` reste bas compare a Step5AC et `DD=0`: le probleme principal n'est pas la compression musicale generale ni le DMC, mais la saturation AUX par les notes NOISE.
- Le glide P2 etait absent cote ROM: P2 avait une structure de glide, mais `tick_pulse_glide(&p2_pitch)` et le test de transport correspondant n'etaient pas actifs, et l'auto-glide etait limite a P1.

## Changements

- Nouveau target short-command: `midi-v2b10`.
- Pico: `arduino\PicoNesV2B_Step5AFNoiseBoundP2Glide`.
- ROM: `Project-V2-A\v2b_step5af_noise_bound_p2_glide`.
- Firmware: `FW=t61-v2b-step5af-noisebound`.
- `NOISE_MAX_PENDING_EVENTS=8`: au-dela, une nouvelle note/gate NOISE remplace le dernier etat NOISE en attente au lieu d'empiler jusqu'a l'overflow.
- Nouveau compteur heartbeat `ND=`: nombre de compressions volontaires de vieux etats NOISE.
- P1/P2/TRI restent exactes: pas de remplacement musical ajoute sur ces voix.
- Controls latest-value-wins conserves, mais injection un peu plus prudente: `12 ms`, seulement quand `Q<=2`.
- Glide P2 retabli cote ROM avec `P2_GLIDE_STEP=24`, tick actif, et transport pitch tick actif.
- Correctif TRI/DMC conserve: `update_triangle()` ne touche pas `$4015`; seul `trigger_dmc()` manipule l'activation DMC.

## A verifier au test

- En full-lane, `N` devrait rester proche de `8` au lieu de monter vers `48`.
- `OVF` devrait rester a `0`.
- `ND` peut monter pendant un pattern NOISE tres dense; c'est normal et indique que les vieux etats NOISE sont compresses au lieu de casser tout le transport.
- Si `ND` monte trop vite et que la partie NOISE perd trop de detail audible, essayer `NOISE_MAX_PENDING_EVENTS=10` ou `12`.
- Le glide P2 doit redevenir audible quand deux notes P2 se suivent legato.
