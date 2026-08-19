# V2B Step5AH Stream Fast Audio Guard ROM

Base: Step5AG stream 5-lane + event.

Objectif: garder le gain de stabilite Step5AG, mais reduire le jitter P1/P2, stabiliser LFO/glide, et empecher les controles de voler du temps aux notes en full-lane.

Changements par rapport a Step5AG:

- Meme protocole stream: `STATUS` annonce `P1/P2/TRI/NOISE/EVENT`, puis la ROM lit `READ_DATA` plusieurs fois dans l'ordre fixe.
- La ROM remplace le gros `delay_short(1)` de chaque lecture bus par un settle court en NOPs.
- Le moteur audio/LFO recoit du temps selon le nombre de lanes traitees, pas seulement le nombre de paquets stream.
- Les controles restent latest-value-wins mais n'entrent sur EVENT que quand les queues live sont vides (`Q=0`) et au plus toutes les `24 ms`.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.
- P1/P2/TRI restent exactes: pas de compression musicale sur ces voix.
- NOISE conserve sa propre phase courte 5 bits et son cap a 24 evenements.
- Le glide P2 reste actif cote ROM.

Pico associe:

- `arduino\PicoNesV2B_Step5AHStreamFastAudioGuard`
- short-command target: `midi-v2b12`
