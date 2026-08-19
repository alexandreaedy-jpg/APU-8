# V2B Step5AM - Slider Diag Wave Anti-Pop

Date: 2026-05-27

Base: Step5AL `midi-v2b16` / `FW=t67-v2b-step5al-linvolwave`.

## Diagnostic

- V/A/D/R ont tres peu de marge; le max semble atteint vers 10% de course.
- Les potards rotatifs sont OK, donc le probleme semble specifique aux sliders.
- Il faut voir le raw MCP pour savoir si le signal sature electriquement ou si une courbe logicielle suffit.
- Le changement waveform ne devait pas dependre du selecteur LFO Pitch/Duty/Amp.
- Des clics restent audibles sur les pulses pendant certains glides.

## Changements

- Nouveau firmware: `FW=t68-v2b-step5am-sliderdiag`.
- Heartbeat `PR=` active en permanence:
  - `PR=` donne les raw filtres des 8 canaux MCP3008 en hexadecimal.
  - Ordre: `Attack, Volume, Decay, Release, LFO Depth, LFO Rate, Duty, CH7`.
- V/A/D/R passent sur une courbe provisoire anti-hot.
- `CH7` en mode ARP OFF applique waveform meme si aucun selecteur LFO Pitch/Duty/Amp n'est actif.
- ROM: duck de 2 ticks apres ecriture mid-note `$4003/$4007`.

## A verifier

- Log: `FW=t68-v2b-step5am-sliderdiag`.
- Lire `PV=` et surtout `PR=`.
- Tester chaque slider lentement de 0 a 100%.
- Si `PR` atteint deja `3F0..3FF` vers 10% de course, le slider sature cote hardware et aucune courbe ne pourra reconstruire la position restante.
- Si `PR` monte progressivement, ajuster la table anti-hot.
- Tester waveform avec ARP OFF et CH7.
- Tester glides pulse: les clics doivent etre moins presents, au prix possible d'un micro-duck.
