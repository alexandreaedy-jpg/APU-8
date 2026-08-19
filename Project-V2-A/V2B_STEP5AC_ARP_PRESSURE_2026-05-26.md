# V2B Step5AC - ARP Pressure

## Pourquoi

Le test `STEP2B-Panel_Control_test20.txt` sur Step5AB montre encore une instabilite quand tout joue en meme temps. Le transport ne deborde pas (`OVF=0`) et `QH` reste modere, mais les files TRI/P1/DMC gardent parfois des etats trop anciens pendant l'arp.

## Changements

### Pico

- Firmware: `t58-v2b-step5ac-arppressure`.
- Nouveau target court: `midi-v2b7`.
- Les voix avec arp actif utilisent une logique latest-state-wins: un nouveau pas d'arp ou un note-off d'arp remplace le dernier etat compact deja en attente sur la meme voix.
- Les note-off/all-notes-off/stop d'arp passent aussi par cette logique, pour eviter une traine de gate-off stale.
- Les nouveaux triggers DMC sont ignores seulement si un arp joue reellement et si la pression musicale atteint `DMC_ARP_PRESSURE_DROP_Q=6`.

### ROM

- Meme base ROM que Step5AB: `SERVICE_AUDIO_SLICE=2`.
- Le protocole `AUX` reste inchange.

## Invariants

- Step5AA reste le rollback-safe base.
- Step5AB reste disponible comme comparaison via `midi-v2b6`.
- Le correctif critique TRI/DMC reste conserve: `update_triangle()` ne touche pas `$4015`; seul `trigger_dmc()` controle le bit DMC.
- DMC reste one-shot trigger-only sur `CH11`.
- Aucun DMC pitch/control continu n'est reactive.

## Tests prioritaires

Resultat `STEP2B-Panel_Control_test21.txt`: rejetee. `MX` monte beaucoup trop vite et les notes TRI/autres voix sont mangees. Ne pas continuer cette branche pour le jeu MIDI exact.

1. `midi-v2b7` full lane + arp P1/P2/TRI:
   - verifier si le groove tient mieux que `midi-v2b6`
   - accepter quelques `DD` sous charge extreme, mais pas de retard musical evident

2. `midi-v2b7` full lane + DMC:
   - verifier que les samples restent presents sans relancer le bug TRI/DMC
   - si `DD` grimpe trop vite, remonter le seuil `DMC_ARP_PRESSURE_DROP_Q`

3. `midi-v2b7` full lane + LFO:
   - verifier que le LFO reste plus regulier quand les 5 voix jouent ensemble
