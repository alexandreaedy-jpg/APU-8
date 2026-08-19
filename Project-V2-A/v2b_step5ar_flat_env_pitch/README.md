# V2B Step5AS Panel Env ROM

Base: Step5AQ longer env.

Objectif: decorreler decay/release, rendre le decay plat, et reduire les hachures pitch quand glide et LFO sont pousses.

Changements par rapport a Step5AQ:

- Pico: zone haute des sliders elargie pour que le niveau `15` soit stable sans exiger `1023` exact.
- Pico: `PR=` reste force dans le heartbeat pour verifier raw et quantification apres test.
- ROM: decay 1-11 devient un hold plat a volume plein, puis le release prend le relais; decay 12-15 tient le plateau a 100% jusqu'au note-off, puis le release prend le relais.
- ROM: decay max augmente a `168` ticks, soit +200% par rapport au max Step5AQ `56`.
- ROM: release ne depend plus du niveau courant, donc il n'est plus raccourci par un decay court.
- ROM: release utilise une table progressive plus douce au debut: les premiers crans sont tres courts, la fin de course reste longue.
- Pico: heartbeat ajoute `VC=` pour afficher l'etat A/D/V/R reellement retenu cote scheduler pour la voix ciblee; `PV` seul n'est pas une preuve que la NES a recu le controle.
- ROM: attaque plus progressive via table de durees.
- ROM: anti-hachure pitch P1/P2: les petits croisements de high-byte dus au LFO/glide sont deferes et le duck high-byte passe de 2 a 1 tick.
- Le correctif TRI/DMC est conserve: TRI ne touche pas `$4015`.

Pico associe:

- `arduino\PicoNesV2B_Step5ARFlatEnvPitch`
- firmware: `FW=t75-v2b-step5as-panelenv`
- short-command target: `midi-v2b22`
