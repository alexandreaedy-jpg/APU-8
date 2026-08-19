# V2B Step5AK - Volume Curve

Date: 2026-05-27

Base: Step5AJ `midi-v2b14` / `FW=t65-v2b-step5aj-polydelay`.

## Diagnostic

- Le potard volume vient d'etre ressoude.
- Le ressenti actuel: volume maximum atteint vers 10% de course.
- L'ancien firmware utilisait une quantification lineaire pour `MCP3008 CH1` volume.
- Il conservait aussi un vieux garde-fou `Volume=0 -> 1` datant du probleme hardware.

## Changements

- Nouveau firmware: `FW=t66-v2b-step5ak-volcurve`.
- Ajout d'une courbe dediee `panelVolumeFromRaw()`:
  - bas de course fortement calme;
  - montee progressive;
  - niveau `15` uniquement en fin de course.
- `MCP_CH_VOLUME` utilise cette nouvelle courbe.
- Suppression du clamp panel `Volume=0 -> 1`.
- ROM audio identique a Step5AJ.

## A verifier

- Log: `FW=t66-v2b-step5ak-volcurve`.
- Heartbeat `PV=`: le second chiffre correspond au volume panel.
- Le volume doit maintenant traverser plusieurs niveaux sur toute la course.
- Si `PV` saute encore directement a `15` vers 10%, le raw MCP sature cote hardware et il faudra lire le raw panel.
