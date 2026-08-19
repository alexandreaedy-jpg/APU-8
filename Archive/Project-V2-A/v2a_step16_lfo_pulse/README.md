# V2_A Step 16 - LFO pulse

Objectif: ajouter un premier LFO stable sur P1/P2 sans revenir au jitter du port manette.

La carte Proto1 reste identique a Step13:

- `0x0`: duty global P1/P2.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

La ROM conserve le moteur Step14/15 valide: pulses amorces, triangle, noise et DMC samples.

Le sketch Pico Step16 conserve la sequence musicale Step15.

Le LFO est volontairement interne ROM pour cette etape:

- Pas de nouveau registre de transport.
- Modulation P1/P2 uniquement.
- Ecriture principalement sur `$4002/$4006` pour eviter de relancer les enveloppes via `$4003/$4007`.

Si cette etape est audible et stable, l'etape suivante pourra exposer depth/rate comme controles lents.
