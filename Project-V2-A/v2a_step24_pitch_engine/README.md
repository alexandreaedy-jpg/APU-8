# V2_A Step 24 - Pulse pitch engine

Objectif: garder le full-lane + LFO musical valide en Step23, mais centraliser le pitch P1/P2 avant d'ajouter glide, pitch bend et arp.

La carte Proto1 reste identique a Step23:

- `0x0`: duty global P1/P2 + depth/rate LFO.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

Changement Step24:

- P1/P2 passent par une structure `PulsePitch`.
- Le timer final est calcule depuis `base_timer + lfo + bend + glide + detune`.
- Pour l'instant, seul `lfo` est actif; les autres offsets restent a zero.
- Les writes HI restent forces au trigger pour conserver les retriggers NES.

Comportement attendu: identique a Step23 a l'oreille. Cette etape sert surtout a rendre la suite plus sure.
