# V2B Step5AS Panel Env - 2026-05-28

Base: Step5AQ / `midi-v2b21`.

Target: `midi-v2b22`

Firmware: `FW=t75-v2b-step5as-panelenv`

Objectif:

- Garder la stabilite Step5AP/Step5AQ avec le hardware sliders 22k/10k.
- Augmenter encore les fins de course Decay/Release.
- Supprimer la correlation audible entre decay court et release raccourci.
- Rendre le decay plat: duree controlee par slider, sans pente descendante.
- Reduire les sons haches/pops quand glide et LFO pitch sont pousses sur P1/P2.

Changements ROM:

- `decay=12..15` tient le plateau a 100% jusqu'au note-off, puis le release prend le relais.
- `decay=1..11` est maintenant un hold a niveau plein, puis le release prend le relais.
- La zone haute des sliders est elargie pour atteindre/stabiliser `15` sans exiger un raw `1023` exact.
- Table decay max: `56` -> `168` ticks, soit +200%.
- Release ne divise plus par le niveau courant.
- Release utilise une table progressive plus douce au debut: premiers crans tres courts, fin de course longue.
- Heartbeat ajoute `VC=` pour comparer les valeurs panel visibles `PV=` avec les valeurs A/D/V/R retenues par le scheduler.
- Attack utilise une table plus progressive.
- Les petits croisements de high-byte P1/P2 dus au LFO/glide sont deferes.
- Le duck de high-byte P1/P2 passe de 2 ticks a 1 tick.

Invariant conserve:

- Le correctif TRI/DMC est conserve. `update_triangle()` ne doit pas ecrire `$4015`.
