# V2B Step5L Panel Fix

Firmware: `t41-v2b-step5l-panelfix`

## Diagnostic du log test4

`PV=` confirme que certaines entrees MCP bougent, mais pas toutes:

- `V` reste bloque a `F`
- `A` reste bloque a `0`
- `D`, `R`, `LD`, `LR`, `DU`, `AR` bougent
- `LM=0` reste bloque, donc le selecteur de cible LFO n'est pas detecte dans `t40`

Conclusion: Volume et Attack ne sont pas recus comme mouvements analogiques par le Pico. Cela pointe vers cablage/mapping MCP3008 ou rails fixes sur ces deux entrees, pas vers le transport NES.

## Changements

- LFO target auto-detecte maintenant deux cablages possibles:
  - commun vers GND avec pull-up interne
  - commun vers 3.3V avec pull-down interne
- `PA=` ne se met plus a jour si le controle analogique a ete ignore. Cela evite de croire qu'un `LD/LR` a ete applique quand `PL=--`.

## A verifier au prochain log

- `LM=` doit devenir:
  - `1` pitch
  - `2` duty
  - `4` amp
- Si `LM` reste `0`, les fils du selecteur LFO ne vont probablement pas aux pins attendues `GP8/GP9/GP15`.
- Si le premier caractere de `PV` reste `F`, le slider Volume est bloque a fond cote MCP.
- Si le deuxieme caractere de `PV` reste `0`, le slider Attack est bloque a zero cote MCP.
