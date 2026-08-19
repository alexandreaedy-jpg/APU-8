# V2B Step5U Fast Groove - 2026-05-22

## Pourquoi cette passe

Le test `STEP2B-Panel_Control_test10` montre que Step5T ne déborde pas (`OVF=0`) mais perd du groove et de la réactivité. Les compteurs `NC` montent très haut, ce qui indique que la fenêtre de note cohort travaille en permanence. Les compteurs `CS` confirment aussi que les contrôles sont coalescés, mais trop retardés.

## Décision

Step5U n'ajoute pas de nouvelle fonction. C'est un rollback sélectif:

- garder les gains Step5T validés: LFO pendant arp, reset LFO côté ROM uniquement sur vraie ouverture de gate, Noise amp/timbre LFO ROM-side, swap panel `CH0=Attack` / `CH1=Volume`
- désactiver le délai artificiel de cohorte de notes
- rendre le scheduler de contrôles moins bloquant pour que `P2` et le panel ne soient plus affamés sous charge

## Changements Pico

- Firmware: `t50-v2b-step5u-fastgroove`
- `enqueueCompactVoiceCohort()` repasse en envoi direct immédiat.
- La constante de fenêtre cohort est mise à `0 us`.
- Le scheduler peut injecter une paire `meta/value` derrière les notes déjà en attente, tant que la queue a la place.
- Le scheduler redevient rotatif sur les voix, au lieu de repartir toujours par `NOISE/P1`.

## Invariants Conservés

- Les notes restent prioritaires: les contrôles s'ajoutent derrière elles, pas devant.
- Pas de drop volontaire de pas d'arp.
- `CH11/DMC` reste désactivé.
- `TRI` reste sans enveloppe et sans glide.
- LFO sous arp reste autorisé pour les voix supportées.

## Test À Faire

- `P1 arp + LFO`: doit rester bon comme Step5T.
- `P2` contrôles: doivent répondre nettement mieux.
- `P1+P2` seulement: le groove ne doit plus traîner.
- `NOISE LFO`: doit rester audible et stable.
- Heartbeat attendu: `FW=t50-v2b-step5u-fastgroove`, `NC=0/0` ou proche de zéro, `OVF=0`.
