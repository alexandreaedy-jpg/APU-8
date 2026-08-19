# V2B Step5F Noise CC Priority - 2026-05-20

## But

Suite au test `STEP2B-TRI_ARP_test14.txt`, `Step5E` ameliore le LFO sous charge mais `NOISE` reste fragile, surtout sur `CC23 release` et `CC24 timbre`.

Constats log:
- `FW=t34-v2b-step5e-fast4`
- `OVF=0`
- `QH` max autour de `13`
- `DMC=--/0`
- `TRI` stable
- gros flux de CC: `MC=5012/133`

Conclusion: pas une collision de voix ni un overflow. Le point faible restant est la reactivite des paires CC `NOISE` sur `AUX`.

## Revision

- Pico firmware: `t35-v2b-step5f-noisectrl`
- ROM cible: `Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi\V2_MIDI_IN_P1P2.nes`
- Short target: `midi-v2b5`

## Changements

### Pico

- Les valeurs de controle deja engagees restent prioritaires.
- Les meta CC `NOISE` passent maintenant devant les rafales de notes `TRI/NOISE`.
- Le score de note `NOISE` est legerement remonte dans l'arbitrage `AUX`.
- `CC24` sur `NOISE` utilise maintenant une valeur 4 bits via `nibbleFromCc()`.
- `CC24` sur `P1/P2` garde l'ancien mapping duty 3 positions.

### ROM

`CC24` pour `NOISE` devient un controle de timbre 4 bits:
- bit 3: mode noise court/long (`$400E` bit 7)
- bits 0..2: offset de periode noise

Le changement de timbre s'applique aussi pendant une note audible, pas seulement au prochain trigger.

## Tests attendus

- Charger `P1 arp + P2 LFO + TRI + NOISE`.
- Bouger `CC23` et `CC24` sur `NOISE` pendant la charge.
- Verifier:
  - `TRI` reste propre.
  - `DMC=--/0`.
  - `OVF=0`.
  - `NOISE release` repond plus vite.
  - `NOISE timbre` devient plus audible et moins bloque.

## Notes

Le `NOISE` reste sur `AUX`, donc a tres haute charge il peut encore prendre de la latence. Cette revision privilegie les controles `NOISE` sans supprimer les pas d'arp et sans reintroduire DMC.
