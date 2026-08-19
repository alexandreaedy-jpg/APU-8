# V2_A Step 23 - Full lane musical LFO

Objectif: reintegrer le LFO musical valide en Step22 dans le full-lane stable.

La carte Proto1 reste identique a Step13:

- `0x0`: duty global P1/P2.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

La ROM conserve le moteur Step15 valide: pulses, triangle, noise et DMC samples.

Ajout Step23:
- `REG_DUTY` reste compatible: bits `0..1` = duty.
- bits `2..4` = profondeur LFO.
- bits `5..7` = rate LFO.
- depth `0` coupe le LFO et revient au comportement Step15.

Le sketch Pico Step15 reduit la pression de queue et envoie des phrases plus proches du futur controle musical:

- P1/P2 notes + triggers groupables.
- TRI en basse tenue.
- NOISE/DMC en ponctuation.
- Duty/ADSR comme controles lents.
