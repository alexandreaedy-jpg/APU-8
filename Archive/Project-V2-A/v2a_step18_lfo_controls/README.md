# V2_A Step 18 - LFO controls

Objectif: exposer rate/depth LFO comme controles lents apres la validation Step17.

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

- Pas de nouvelle phase transport.
- Modulation P1/P2 uniquement.
- Accumulateur fractionnaire + rampe triangulaire fine pour reduire l'effet "stepped".
- `REG_DUTY` devient packe: bits `0..1` duty, bits `2..4` LFO depth, bits `5..7` LFO rate.

Cette etape valide que le Pico peut piloter la modulation sans streamer le LFO lui-meme.
