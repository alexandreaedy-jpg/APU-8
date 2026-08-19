# V2B Step5S - Panel Responsiveness, Percussive Decay, Pulse Anti-Click

Date: 2026-05-21

Firmware Pico: `t48-v2b-step5s-panelenvclick`

## Objectif

Stabiliser le front panel sans changer le transport `P1/P2/TRI/NOISE` valide.

Cette etape garde le profil no-DMC, ignore le slider volume tant que son souci hardware n'est pas regle, rend `Attack/Decay/Release` plus reactifs, transforme `Decay` en enveloppe percussive, et reduit les clicks pulse en evitant les writes inutiles sur `$4003/$4007`.

## Pico

- Remplace l'ancien arbitrage "le plus gros delta gagne" par un verrou de controle actif de `120 ms`.
- Deadbands par famille:
  - `Attack/Decay/Release`: `16`
  - `LFO depth/rate`: `20`
  - `Duty/Timbre` et `Arp Time/LFO Delay`: `32`
- Le slider `Volume` est ignore cote panel pour eviter les injections fantomes pendant diagnostic hardware.
- Apres changement de voix ou cible LFO, le Pico reapplique seulement `LFO depth`, `LFO rate`, et `LFO delay` si l'arp est OFF.
- L'exclusivite LFO est support-aware: une cible LFO non supportee par la voix selectionnee ne coupe plus une modulation valide deja active.
- Les LFO restent neutralises pendant arp actif; les valeurs panel courantes sont reappliquees quand l'arp est desactive.
- `NOISE` accepte maintenant `Attack/Decay/Release/Duty` et `Amp LFO` cote controle, pour correspondre au moteur ROM.

## ROM

- `Decay` ne descend plus vers le niveau `Volume`.
- `Volume` devient un niveau de sortie.
- `Decay 0..14` descend l'enveloppe vers zero, de tres court a long.
- `Decay 15` signifie sustain/held.
- `TRI` reste hors enveloppe.

## Anti-Click Pulse

- Aucun sweep experimental active.
- `P1_SWEEP/P2_SWEEP` restent a `$08`.
- Les writes `$4003/$4007` ne sont plus forces si les high bits du timer n'ont pas change et que le timer high est deja initialise.
- Le premier write reste garanti grace au sentinel `written_hi = 0xFF`.

## Tests Cibles

- `PA=A/D/R` doit suivre plus vite, sans parasites massifs sur les autres canaux.
- Changement voix/cible LFO: depth/rate/delay doivent agir sans avoir a rebouger les pots.
- P1/P2/NOISE avec note tenue: `Decay 0/1` doit faire un pluck court; `Decay 15` doit rester tenu.
- Arp `1/16` et `1/32`: decay doit etre audible sur les steps, sans casser le groove.
- Repetitions rapides P1/P2: moins de clicks/pops, sans notes muettes ni pitch bloque.

## Reference

- NESdev rappelle que les writes high timer pulse resetent le sequencer; Step5S evite donc les writes high inutiles avant toute tentative sweep.
