# V2_A Step 25 - Portamento / glide

Objectif: valider le glide P1/P2 sur le moteur pitch central Step24.

Le protocole Proto1 reste identique:

- `0x0`: duty global P1/P2 + depth/rate LFO.
- `0x1..0x4`: ADSR global attack/decay/sustain/release.
- `0x5..0x7`: TRI note/gate/trigger.
- `0x8..0xA`: P1 note/gate/trigger.
- `0xB..0xD`: P2 note/gate/trigger.
- `0xE..0xF`: NOISE/DMC note/gate.

Changement Step25:

- `PulsePitch` garde maintenant un timer cible et un timer courant.
- Si la note change sans changement de trigger, le timer courant glisse vers la cible.
- Si le trigger change, on garde le comportement Step24: reset envelope + jump direct vers la note.
- Le LFO Step23 reste intact et s'additionne au timer courant.

Pattern Pico attendu:

- P1/P2 seuls au debut.
- Gate tenue, notes changeantes sans trigger: portamento audible.
- Quelques triggers ensuite pour comparer avec des attaques/restarts francs.
