# V2B Step5I Panel Debounce - 2026-05-20

## But

Stabiliser les controles physiques MCP3008 apres le premier test panel.

Le probleme observe sur `STEP2B-Panel_Control_test1` n'etait pas le transport note: le compteur `PC=` montait beaucoup trop vite, signe que le panel reinjectait des etats de controle sans mouvement musical clair.

## Revision

- Pico firmware: `t38-v2b-step5i-paneldebounce`
- ROM: inchangee par rapport a Step5H.
- Heartbeat etendu:
  - `PC=` actions appliquees depuis le panel
  - `PT=` cible voix vue par le Pico: `P1`, `P2`, `TRI`, `NOI`, `GLB`, `--`
  - `PL=` cible LFO vue par le Pico: `PIT`, `DUT`, `AMP`, `--`
  - `PV=` valeurs quantifiees MCP3008, dans l'ordre `V A D R LfoDepth LfoRate Duty ArpTime`

## Changements

### Selecteurs

- Debounce logiciel sur les selecteurs voix et cible LFO.
- Les entrees restent supposees actives a `LOW` avec `INPUT_PULLUP`.
- Si le selecteur semble inverse, verifier d'abord `PT=` et `PL=` dans le heartbeat avant de modifier le mapping.

### Potentiometres

- Scan panel ralenti a `24 ms`.
- Deadband raw augmente a `24`.
- Au boot ou au changement de cible, le Pico lit et memorise les positions, mais n'envoie aucun controle.
- Un controle n'est envoye qu'apres un vrai mouvement au-dela du deadband.

### Changement de cible

Le changement `P1 -> P2`, `P2 -> TRI`, etc. ne force plus l'envoi de tous les potards a la nouvelle voix.

C'est volontaire: le panel agit comme un controle live non motorise. Il faut bouger le potard pour appliquer sa valeur a la nouvelle cible.

## Test recommande

1. Flasher `t38-v2b-step5i-paneldebounce`.
2. Ne rien toucher pendant quelques secondes.
3. Verifier que `PC=` reste stable ou monte tres peu.
4. Tourner le selecteur voix position par position et lire `PT=`.
5. Tourner le selecteur LFO position par position et lire `PL=`.
6. Bouger un seul potard a la fois et verifier `PV=`.
7. Si un controle est inverse, noter quelle position physique donne quelle valeur `PV=`.

## Interpretation rapide

- Si `PT=` ne correspond pas a la position physique, corriger le mapping des pins voix.
- Si `PL=` ne correspond pas, corriger le mapping des pins LFO.
- Si `PV=` bouge sans toucher le panel, augmenter encore deadband/filtrage ou verifier le cablage analogique du MCP3008.
- Si `PV=` est stable mais le son change, le probleme est cote application controle/audio, pas cote lecture panel.
