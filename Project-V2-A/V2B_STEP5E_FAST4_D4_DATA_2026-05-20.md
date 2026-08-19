# V2B Step5E Fast4 D4 Data - 2026-05-20

## But

Stabiliser la charge `P1/P2/TRI/NOISE` apres le profil `Step5D exact4`, maintenant que le cablage `D4 -> GP14` est corrige.

Problemes vises:
- `LFO` moins propre quand `P1/P2` jouent avec d'autres voix.
- `NOISE` garde son groove, mais ses CC `release` et `timbre` reagissent mal sous charge.
- `TRI` est valide et doit rester strictement separe de `NOISE` et du DMC desactive.

## Revision

- Pico firmware: `t34-v2b-step5e-fast4`
- Target court: `midi-v2b5`
- ROM: `Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi\V2_MIDI_IN_P1P2.nes`
- Pico sketch: `arduino\PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi`

## Changement protocole

`D4` n'est plus un bit `valid` permanent. Il devient une vraie cinquieme ligne de donnee.

Statut:
- `D0`: `P1` pending
- `D1`: `P2` pending
- `D2`: `AUX` pending
- `D3`: `AUX is TRI`
- `D4`: `AUX full byte`

Donnees completes:
- `P1`, `P2`, `TRI` restent sur deux lectures: `LO = bits 0..4`, `HI = bits 5..7`.
- `NOISE` controles restent en deux lectures avec `AUX full byte`.

Donnees courtes:
- `NOISE` note/gate passe en une seule lecture `AUX_LO`.
- `D0..D3`: note noise `0..15`
- `D4`: gate noise

Consequence log attendue:
- `D=00000` au repos est normal.
- `AX=NOI:xx` peut etre draine plus vite sur les notes.
- `AX=TRI:xx` reste une lecture complete.
- `DMC=--/0` reste attendu.

## Changement ROM

La ROM lit maintenant le format 5 bits:
- plus de test `STATUS_VALID`.
- reconstruction 8-bit par `5 + 3`.
- lecture `NOISE` courte si `AUX` n'est pas `TRI` et si `AUX_FULL=0`.

Le service transport intercale maintenant `update_audio()` pendant les rafales:
- budget de drain conserve a `64`.
- audio/LFO/enveloppes mis a jour tous les `4` batches traites.
- mise a jour audio finale apres un batch non vide.

Objectif: eviter que le transport monopolise la boucle quand les notes + CC arrivent vite.

## Changement Pico

Le Pico encode:
- `P1/P2/TRI`: 8 bits en `LO5 + HI3`.
- `NOISE note/gate`: format court 5 bits.
- `NOISE CC`: format complet preserve pour ne jamais casser les paires meta/value.

Le DMC reste desactive en live:
- `CH11` ignore.
- aucun trigger DMC dans `AUX`.

## Tests prioritaires

1. `P1 arp 1/16`, puis `1/32`.
2. Ajouter `P2 LFO`.
3. Ajouter `TRI`.
4. Ajouter `NOISE`.
5. Sous charge, bouger `CC23 release` et `CC24 timbre` du `NOISE`.
6. Verifier que `TRI` reste propre, sans DMC, sans sustain infini.

Succes attendu:
- pas de permutation d'instrument.
- pas de DMC parasite.
- `OVF=0` sur rafales courtes.
- `NOISE` CC plus reactifs.
- `LFO` moins perturbe quand une autre voix s'ajoute.

## Rollback

Retour direct possible vers `t33-v2b-step5d-exact4` si le nouveau statut sans `valid` permanent introduit une instabilite.
