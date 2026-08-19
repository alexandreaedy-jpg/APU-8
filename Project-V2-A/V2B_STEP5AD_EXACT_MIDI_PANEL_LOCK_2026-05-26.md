# V2B Step5AD - Exact MIDI / Panel Lock

## Pourquoi

Le test `STEP2B-Panel_Control_test21.txt` sur Step5AC est pire que Step5AB: `OVF=0` et `QH=10`, mais `MX` monte tres vite (`743 -> 1481`). La nouvelle logique Step5AC remplace donc trop d'evenements musicaux avant que la NES les lise, ce qui explique les notes TRI et les autres notes mangees.

## Changements

### Pico

- Firmware: `t59-v2b-step5ad-exact-panel-lock`.
- Nouveau target court: `midi-v2b8`.
- Base: Step5AB, pas Step5AC.
- Les pas d'arp ne remplacent plus de note en attente: les notes restent exactes.
- Le panel analogique est lu mais n'applique plus de changements pendant que le MIDI clock tourne ou qu'une activite MIDI est recente.
- Le heartbeat ajoute `MN=noteOn/noteOff` pour distinguer reception MIDI et transport NES.

### ROM

- Meme ROM que Step5AB: `SERVICE_AUDIO_SLICE=2`.
- Protocole `AUX` inchange.

## Invariants

- Step5AA reste le rollback-safe base.
- Step5AB et Step5AC restent disponibles comme comparaison.
- Le correctif critique TRI/DMC reste conserve: `update_triangle()` ne touche pas `$4015`; seul `trigger_dmc()` controle le bit DMC.
- DMC reste one-shot trigger-only sur `CH11`.

## Tests prioritaires

1. `midi-v2b8` full lane sans toucher au panel:
   - `MX` doit rester beaucoup plus bas que Step5AC
   - `PC`/`CS` doivent rester quasi stables pendant `CLK=ON`
   - `MN=` doit monter quand les notes MIDI arrivent

2. `midi-v2b8` TRI + arp:
   - verifier que les notes TRI ne sont plus mangees
   - accepter un retard eventuel avant toute nouvelle optimisation de groove

3. `midi-v2b8` full lane + LFO:
   - verifier si le LFO reste plus propre quand le panel ne change plus ses controles pendant la lecture
