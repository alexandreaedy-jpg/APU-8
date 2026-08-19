# V2B Step5AE - Hard Realtime Notes

## Pourquoi

Objectif: garder des notes MIDI exactes sous full lane, tout en retrouvant une reactivite CC/panel. Step5AD protegeait les notes mais gelait trop le panel. Step5AE garde les controles latest-value-wins, mais donne aux notes une priorite dure dans les files.

## Changements

### Pico

- Firmware: `t60-v2b-step5ae-hardrt`.
- Nouveau target court: `midi-v2b9`.
- Les notes compactes ne sont plus remplacees par des notes plus recentes.
- Chaque evenement de voix garde un type: note, controle ou DMC.
- Chaque evenement garde un timestamp d'entree en file.
- Une note peut passer devant un controle meta/value non commence dans la meme file.
- Si le controle value est deja en tete, il est termine avant la note pour ne pas casser une paire meta/value deja exposee a la NES.
- Sur `AUX`, les controles meta TRI/NOISE ne passent plus devant une tete musicale TRI/NOISE/DMC.
- Le scheduler controle est plus reactif: une paire toutes les `8 ms` si `Q<=3`.
- Le panel n'est plus gele pendant le MIDI clock; il met a jour les slots latest-value-wins.
- Le heartbeat ajoute:
  - `MN=noteOn/noteOff recus`
  - `NL=maxLatencyMs/delayedCount`
  - `CL=maxControlLatencyMs`

### ROM

- Meme ROM que Step5AB/AD: `SERVICE_AUDIO_SLICE=2`.
- Protocole `AUX` inchange.

## Invariants

- Step5AA reste le rollback-safe base.
- Step5AC reste rejetee pour jeu MIDI exact.
- Le correctif critique TRI/DMC reste conserve: `update_triangle()` ne touche pas `$4015`; seul `trigger_dmc()` controle le bit DMC.
- DMC reste one-shot trigger-only sur `CH11`.

## Tests prioritaires

1. `midi-v2b9` full lane notes seules:
   - `MN` monte avec les notes recues
   - `NL` doit rester bas
   - les notes TRI ne doivent plus etre mangees

2. `midi-v2b9` full lane + CC MIDI:
   - les controles doivent bouger sans faire exploser `NL`
   - `CL` peut monter, c'est acceptable si le son suit sans casser le groove

3. `midi-v2b9` full lane + panel:
   - `PC/CS` doivent reagir
   - `MX` peut monter pour les controles latest-value-wins, mais les notes ne doivent pas disparaitre
