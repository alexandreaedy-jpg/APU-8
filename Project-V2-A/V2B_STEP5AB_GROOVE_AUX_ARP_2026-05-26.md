# V2B Step5AB - Groove AUX/ARP

## Pourquoi

Step5AA est propre cote DMC, mais le test full-lane montre une perte de groove quand plusieurs voix jouent ensemble, surtout avec l'arp. Le log `STEP2B-Panel_Control_test19.txt` indique `OVF=0`, donc le probleme le plus probable n'est pas une queue qui explose, mais des evenements qui arrivent trop tard ou par paquets.

## Changements

### Pico

- Firmware: `t57-v2b-step5ab-groove`.
- Nouveau target court: `midi-v2b6`.
- `AUX` utilise vraiment l'arbitrage rotatif TRI/NOISE/DMC deja present dans le code.
- TRI ne peut plus monopoliser `AUX` quand NOISE attend aussi.
- DMC reste moins prioritaire que les notes, mais peut encore s'intercaler apres le petit seuil de deferral Step5AA.
- Les pas d'arp remplacent un vieux gate-on en attente sur la meme voix au lieu d'empiler des pas stale; si le bus ne suit pas, on prefere sauter un pas plutot que jouer l'arp en retard.
- Le diagnostic `auxVoicesContending()` compte maintenant DMC aussi.

### ROM

- `SERVICE_AUDIO_SLICE` passe de `4` a `2` pendant les bursts transport.
- Objectif: reduire la famine LFO/enveloppe sous charge, sans changer le protocole.

## Invariants

- Step5AA reste le rollback-safe base.
- DMC reste one-shot trigger-only sur `CH11`.
- `AUX` reste type par status bits: TRI / DMC / NOISE full / NOISE short.
- Aucun DMC pitch/control continu n'est reactive.

## Tests prioritaires

1. `midi-v2b6` full lane sans arp:
   - comparer groove vs `midi-v2b5`
   - attendre `OVF=0`
   - `QH` peut monter, mais les notes ne doivent pas arriver en retard audible

2. `midi-v2b6` arp P1/P2/TRI:
   - verifier que l'arp garde le tempo
   - accepter que certains pas soient sautes sous charge extreme plutot que retardes

3. `midi-v2b6` full lane + LFO:
   - verifier si le LFO reste plus regulier quand P1/P2/TRI/NOISE/DMC jouent ensemble

