# V2B Step5AK Volume Curve ROM

Base: Step5AJ poly/delay/release/anti-pop.

Objectif: recalibrer la course du potard volume apres ressoudage.

Changements par rapport a Step5AJ:

- Pico: nouvelle courbe dediee `panelVolumeFromRaw()` pour `MCP3008 CH1`.
- Le volume panel n'utilise plus la courbe lineaire `0..1023 -> 0..15`.
- Les premiers 10-20% de course restent bas au lieu d'atteindre rapidement le maximum.
- Le niveau `15` n'arrive qu'en toute fin de course.
- Suppression du vieux clamp panel `Volume=0 -> 1`, car le potard volume est repare.
- ROM identique a Step5AJ pour le moteur audio.

Pico associe:

- `arduino\PicoNesV2B_Step5AKVolumeCurve`
- firmware: `FW=t66-v2b-step5ak-volcurve`
- short-command target: `midi-v2b15`
