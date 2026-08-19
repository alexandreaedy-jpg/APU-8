# V2B Step5AE Hard Realtime Notes ROM

Base: Step5AD exact MIDI / panel lock.

Objectif: notes prioritaires et non destructives, avec CC/panel coalesces et injectes sans casser le groove.

Changements par rapport a Step5AD:

- Meme ROM que Step5AB: `SERVICE_AUDIO_SLICE=2`.
- Protocole inchange: P1/P2 lanes dediees, TRI/NOISE/DMC via `AUX` type.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.
- Les notes passent devant les controles non commences dans la file de voix.
- Les controles restent latest-value-wins et peuvent etre injectes plus reactifs (`8 ms`, `Q<=3`).
- Le heartbeat ajoute `NL=max/delayed` et `CL=max`.

Pico associe:

- `arduino\PicoNesV2B_Step5AEHardRealtimeNotes`
- short-command target: `midi-v2b9`
