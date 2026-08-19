# V2_A Step 17 - LFO smooth

Objectif: rendre le LFO P1/P2 plus musical apres la validation Step16.

La carte Proto1 reste identique a Step13:

- `0x0`: duty global P1/P2.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

La ROM conserve le moteur Step14/15 valide: pulses amorces, triangle, noise et DMC samples.

Le sketch Pico Step16 conserve la sequence musicale Step15.

Le LFO reste volontairement interne ROM pour cette etape:

- Pas de nouveau registre de transport.
- Modulation P1/P2 uniquement.
- Accumulateur fractionnaire + rampe triangulaire fine pour reduire l'effet "stepped".

Si cette etape est audible et stable, l'etape suivante pourra exposer depth/rate comme controles lents.
