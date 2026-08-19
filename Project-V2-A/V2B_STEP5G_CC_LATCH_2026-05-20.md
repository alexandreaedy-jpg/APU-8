# V2B Step5G CC Latch - 2026-05-20

## But

Reduire les actions CC inutiles sous charge, surtout les controles de type interrupteur qui peuvent envoyer plusieurs messages identiques.

Le principe est le mode `sample and hold`:
- le Pico garde l'etat courant en memoire;
- la ROM garde l'etat applique;
- aucun transport NES n'est emis tant qu'une nouvelle valeur quantifiee ne change pas vraiment l'etat.

## Revision

- Pico firmware: `t36-v2b-step5g-cclatch`
- Base: `t35-v2b-step5f-noisectrl`
- ROM: inchangee cote protocole/audio, reconstruite pour garder l'artefact courant.

## Changements Pico

### CC continus

Les controles `attack`, `decay`, `volume`, `release`, `duty/timbre`, `LFO depth`, `LFO rate`, `LFO delay`, `LFO wave` etaient deja largement proteges:
- conversion en valeur compacte;
- comparaison avec l'etat memorise;
- emission uniquement si la valeur change.

Cette logique reste la base.

### CC On/Off: Arp Enable

Avant, un `CC27` repete pouvait relancer la logique arp meme si l'etat etait deja `ON` ou deja `OFF`.

Maintenant:
- `OFF -> OFF`: ignore;
- `ON -> ON`: ignore;
- `OFF -> ON`: applique une seule fois;
- `ON -> OFF`: applique une seule fois.

Consequence attendue:
- plus de reset arp parasite si un controleur MIDI renvoie regulierement son etat.
- `MC applied` baisse si le controleur spamme `CC27`.

### CC de mode: Arp Division

Avant, un `CC28` repete dans la meme zone pouvait remettre `clockCounter` a zero.

Maintenant:
- meme division: ignore;
- nouvelle division: applique et reset le compteur une seule fois.

Consequence attendue:
- moins de micro-decrochages arp dus a des messages de division repetes.

## Log attendu

- `FW=t36-v2b-step5g-cclatch`
- `MC received` peut rester haut si le controleur MIDI spamme.
- `MC applied` doit moins monter pendant les CC repetes.
- `Q/QH/OVF` ne doivent pas monter a cause de CC identiques.

## Test recommande

1. Tenir un arp `P1` avec clock.
2. Laisser le controle `CC27` sur `ON` et verifier qu'il ne redemarre pas l'arp.
3. Bouger `CC28`, puis rester dans la meme division: pas de decrochage.
4. Ajouter `P2 LFO`, `TRI`, puis `NOISE`.
5. Sous charge, bouger `CC23` et `CC24` du `NOISE`.

Succes:
- groove arp plus stable avec des controles qui renvoient leur etat.
- `NOISE` toujours perfectible sous charge extreme, mais sans resets CC inutiles.
