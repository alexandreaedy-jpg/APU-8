# V2_A Step 15 - Music control

Objectif: passer de la preuve full lane Step14 a une sequence musicale plus lisible.

La carte Proto1 reste identique a Step13:

- `0x0`: duty global P1/P2.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

La ROM conserve le moteur Step14 valide: pulses amorces, triangle, noise et DMC samples.

Le sketch Pico Step15 reduit la pression de queue et envoie des phrases plus proches du futur controle musical:

- P1/P2 notes + triggers groupables.
- TRI en basse tenue.
- NOISE/DMC en ponctuation.
- Duty/ADSR comme controles lents.
