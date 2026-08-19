# V2_A MIDI In P1/P2 ROM

Base: Step25 portamento/glide.

Objectif: ROM silencieuse au boot, pilotee par le sketch Pico `PicoNesV2A_MidiInP1P2`.

Changement par rapport a Step25:

- P1 et P2 demarrent avec `gate=0`.
- La ROM ne joue plus de note par defaut sans evenement de controle.
- Le moteur pitch central, le LFO et le portamento restent disponibles.

Comportement attendu:

- Silence tant qu'aucun message MIDI valide n'arrive au Pico.
- `Note On` MIDI -> P1 puis P2.
- `Note Off` MIDI -> gate off de la voix correspondante.
