# V2B Step5M Panel Calibration

Firmware: `t42-v2b-step5m-panelcal`

## Diagnostic test5

Le log `STEP2B-Panel_Control_test5.txt` montre:

- `V` reste bloque a `0` pendant tout le test.
- `A` saute entre `0` et des valeurs tres hautes.
- `LM` detecte bien les trois cibles LFO:
  - `1` pitch
  - `2` duty
  - `4` amp
- La detection LFO etait cependant instable car le firmware alternait entre pull-up et pull-down.

## Changements

- Le selecteur LFO est maintenant lu en active-high fixe avec pull-down interne.
- La courbe `Attack` est fortement adoucie:
  - grande zone basse a zero
  - montee cubique sur la fin de course
- `Volume=0` venant du panneau est ignore pour eviter un mute fantome quand CH0 est colle a la masse.

## A verifier

- `PL` doit pouvoir revenir a `PIT` de maniere stable.
- `Attack` doit etre beaucoup moins violent en debut de course.
- Si `PV` premier caractere reste `0`, le slider Volume n'est toujours pas lu par le MCP3008 CH0.
