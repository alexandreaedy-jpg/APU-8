# V2_A Step 20 - Control full LFO

Objectif: revenir au full-lane musical apres validation du LFO isole.

La carte Proto1 reste identique a Step13:

- `0x0`: duty global P1/P2.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

La ROM conserve le moteur Step14/15 valide: pulses amorces, triangle, noise et DMC samples.

Le sketch Pico Step20 utilise le meme `ControlState` interne:

- notes P1/P2/TRI.
- gate/trigger.
- ADSR.
- duty + LFO depth/rate packed.
- noise et DMC comme evenements rythmiques.

Le LFO reste interne ROM:

- Pas de nouvelle phase transport.
- Modulation P1/P2 uniquement.
- Rampe triangulaire 8-bit plus fine que Step19 diagnostic.
- Ecriture timer pulse corrigee: `LO` + `HI` seulement quand le high byte change.
- `REG_DUTY` devient packe: bits `0..1` duty, bits `2..4` LFO depth, bits `5..7` LFO rate.

Cette etape prepare le branchement MIDI: la future entree MIDI remplira le meme etat au lieu de remplacer le transport.
