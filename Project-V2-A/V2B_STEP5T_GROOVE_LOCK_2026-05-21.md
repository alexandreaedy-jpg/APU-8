# V2B Step5T - Groove Lock, LFO Sous Arp, Scheduler Controles

Date: 2026-05-21

Firmware Pico: `t49-v2b-step5t-groovelock`

## Objectif

Retrouver un groove stable sous charge en supprimant les rafales de controles, en gardant les LFO actifs pendant l'arp, et en groupant mieux les notes MIDI quasi simultanees.

## Pico

- `MCP3008 CH0` controle maintenant `Attack`.
- `MCP3008 CH1` controle maintenant `Volume`.
- Le volume panel est reactif a nouveau; `PA=` affiche la fonction logicielle reelle.
- Les LFO ne sont plus mutes quand l'arp est active.
- Les controles LFO ne sont plus bloques pendant l'arp.
- Les controles voix sont coalesces dans un scheduler `latest value wins`.
- Le scheduler injecte au maximum une paire `meta/value` controle a la fois.
- Les notes/arp restent prioritaires; les controles attendent si leur lane voix est occupee.
- Les controles `NOISE` gardent la priorite de reaction, mais ne volent plus durablement le flux aux notes.
- Ajout d'une micro-fenetre de cohorte note d'environ `1200 us` pour exposer des notes de voix differentes dans le meme snapshot transport.
- Heartbeat:
  - `CS=pending/injected`
  - `NC=active_or_pending/flushed`

## ROM

- Les LFO ne redemarrent plus a chaque step d'arp.
- Le reset LFO/delay ne se fait plus que lorsque la voix etait vraiment silencieuse avant le nouveau gate.
- `NOISE` accepte `CTRL_LFO_DUTY_DEPTH` comme modulation locale du timbre.
- Le timbre LFO du Noise module le period uniquement, avec clamp `0..15`.
- Le bit mode Noise n'est pas module par le LFO.
- `TRI LFO` reste desactive volontairement.

## Contraintes

- DMC reste desactive.
- Aucun flux continu de modulation n'est envoye depuis le Pico.
- Les controles ne doivent jamais produire de gate-off.
- En cas de charge forte, les controles doivent attendre ou etre remplaces, pas supprimer des notes.

## Tests Cibles

- Slider 1: attaque.
- Slider 2: volume.
- `P1` arp + LFO pitch/duty/amp: LFO continu, pas de reset a chaque step.
- Accords exacts `P1+P2`, `P1+TRI`, `P2+NOISE`: moins de flam/micro-decalage audible.
- `NOISE` timbre manuel + amp LFO + timbre LFO: pas de blocage durable, pas de passage muet.
- Charge extreme: `P1 arp 1/32 + P2 LFO + TRI arp + NOISE controles`.
- Suivre `OVF=0`, `Q`, `QH`, `CS=`, `NC=`.
