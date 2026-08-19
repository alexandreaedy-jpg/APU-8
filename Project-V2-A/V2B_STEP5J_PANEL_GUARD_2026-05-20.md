# V2B Step5J Panel Guard - 2026-05-20

## But

Corriger les melanges de sliders observes dans `STEP2B-Panel_Control_test2`.

Le transport note reste inchange. Cette etape ne touche qu'a la lecture et a l'application des controles physiques MCP3008.

## Revision

- Pico firmware: `t39-v2b-step5j-panelguard`
- ROM: inchangee.

## Probleme observe

`PV=` montre des changements rapides sur plusieurs canaux analogiques, et `PC=` monte par paquets.

Sur un panel proto, cela peut arriver par:
- bruit analogique;
- diaphonie entre fils;
- masse ou reference analogique imparfaite;
- mouvement de potard lu sur plusieurs scans proches;
- selecteurs qui changent la cible pendant qu'une valeur analogique bouge.

Avant Step5J, chaque canal MCP modifie dans un scan pouvait generer un controle reel.

## Changements

### Geste dominant

A chaque scan panel:
- le Pico detecte tous les canaux analogiques qui ont change;
- il choisit uniquement le canal avec le plus gros delta raw;
- seul ce controle est applique;
- les autres changements du meme scan sont latches mais non envoyes.

Effet attendu:
- un slider ne doit plus declencher plusieurs controles en meme temps;
- moins de conflits V/A/D/R;
- moins de CC fantomes sur `NOISE`;
- un geste physique = un controle musical.

### Filtrage

- Deadband raw: `32`.
- Scan panel: `24 ms`.
- Step5I debounce des selecteurs conserve.
- Changement de cible voix/LFO conserve en mode latch: aucune valeur analogique n'est envoyee tant que le potard ne bouge pas.

### Heartbeat

Champs utiles:
- `PV=` valeurs quantifiees `V A D R LfoDepth LfoRate Duty ArpTime`.
- `PA=` dernier controle analogique applique:
  - `V`
  - `A`
  - `D`
  - `R`
  - `LD`
  - `LR`
  - `DU`
  - `AR`
- `PS=` nombre de changements analogiques absorbes car non dominants.

## Test recommande

1. Selectionner `P1`.
2. Bouger uniquement `Volume`, puis verifier que `PA=V`.
3. Bouger uniquement `Attack`, puis verifier que `PA=A`.
4. Repeter pour `Decay`, `Release`, `LD`, `LR`, `DU`, `AR`.
5. Refaire sur `P2`, puis `NOISE`.
6. Si `PA=` ne correspond pas au controle physique bouge, le cablage MCP3008 ou l'ordre des canaux doit etre remappe.

## Interpretation

- `PS=0` ou bas: panel propre.
- `PS` monte pendant un mouvement: le garde-fou absorbe bien des changements parasites.
- `PV` bouge sur un canal qui n'est pas touche: verifier cablage, masse, VREF, ou blindage/longueur des fils.
