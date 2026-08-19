# V2_A Step 13 - Full lane queue smoke test

Objectif: valider dans une seule ROM les familles deja confirmees separement:
P1, P2, TRI, NOISE, DMC samples, duty et ADSR global.

Carte des 16 registres Proto1:

- `0x0`: duty global P1/P2.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

Les notes NOISE `36/38/41/46` declenchent les samples DMC kick/snare/rim/voice.
