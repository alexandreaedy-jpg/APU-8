# V2B Step5AA - DMC One-Shot AUX

## Pourquoi

Step5Z a retire l'ambiguite de decodage `AUX`, mais le test hardware montrait encore des samples DMC au note-on/note-off du `TRI`.

Le nouveau diagnostic pointe vers `$4015`: sur NES, remettre le bit DMC a 1 peut redemarrer le sample si le compteur DMC est vide. Tant que `update_triangle()` reecrivait `$4015` avec le bit DMC conserve, un changement de gate `TRI` pouvait relancer le dernier sample sans aucun nouveau message `CH11`.

Reference utile: https://www.nesdev.org/wiki/APU_Status

## Changements

### ROM

- `update_triangle()` ne touche plus `$4015`.
- `TRI` est coupe uniquement par `TRI_LINEAR = 0`.
- `trigger_dmc()` est le seul chemin qui clear/set le bit DMC:
  - `$4015 = $0F`
  - ecriture `$4010/$4011/$4012/$4013`
  - `$4015 = $1F`
- Le type `AUX=DMC` declenche directement `trigger_dmc_id(value)`.
- L'ancien protocole `FD/FE + E0..EF` n'est plus requis pour DMC.

### Pico

- Firmware: `t56-v2b-step5aa-dmconeshot`.
- `CH11` transporte un seul byte `sample_id 0..25` sur `AUX=DMC`.
- La queue DMC est limitee a 4 evenements.
- Le DMC n'est plus droppe juste parce qu'une autre voix musicale est pending.
- Si `TRI/NOISE` occupent `AUX`, le DMC attend un petit nombre de tours, puis s'intercale.

## Invariants

- `P1/P2` restent inchanges.
- `TRI/NOISE/DMC` restent explicitement types par le status `AUX`.
- `NOISE` note/gate reste compact.
- `NOISE` controles restent full byte.
- `DMC pitch` reste desactive.

## Tests Prioritaires

1. `TRI` seul, aucun clip MIDI sur `CH11`:
   - aucune percussion au note-on
   - aucune percussion au note-off
   - log attendu: `DMC=--/0`

2. `DMC` seul sur `CH11`:
   - samples Amen `36..61`
   - log attendu: `AX=DMC:00..19`

3. `TRI + DMC`:
   - les samples ne partent que sur les notes `CH11`
   - `TRI` ne redemarre plus le dernier sample

4. Full lane:
   - `P1/P2/TRI/NOISE + DMC`
   - les samples peuvent etre retardes si `AUX` est charge, mais ne doivent pas etre confondus avec `TRI`

