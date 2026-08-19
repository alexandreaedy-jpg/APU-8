# V2B Step5AG Stream 5-Lane + Event ROM

Base: Step5AF noise bound + P2 glide.

Objectif: sortir TRI et NOISE de l'ancien AUX partage, puis reserver une lane EVENT pour DMC + controles.

Changements par rapport a Step5AF:

- Meme ROM que Step5AB: `SERVICE_AUDIO_SLICE=2`.
- Nouveau protocole stream: `STATUS` annonce `P1/P2/TRI/NOISE/EVENT`, puis la ROM lit `READ_DATA` plusieurs fois dans l'ordre fixe.
- Lanes notes separees: `P1`, `P2`, `TRI`, `NOISE`.
- Lane `EVENT`: DMC + controles CC/panel coalesces.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.
- P1/P2/TRI restent exactes: pas de compression musicale sur ces voix.
- NOISE a sa propre phase courte 5 bits et ne partage plus ses slots avec TRI.
- Le cap NOISE est remonte a 24 evenements en attente, car la nouvelle lane reduit fortement la pression.
- Le glide P2 reste actif cote ROM.

Pico associe:

- `arduino\PicoNesV2B_Step5AGStream5LaneEvent`
- short-command target: `midi-v2b11`
