# V2_A Step 14 - Full lane play

Objectif: passer du smoke test Step13 a une sequence continue plus musicale.

La carte Proto1 reste identique a Step13:

- `0x0`: duty global P1/P2.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

La ROM conserve le moteur Step13 corrige: pulses maintenus en actif, triangle, noise et DMC samples.
