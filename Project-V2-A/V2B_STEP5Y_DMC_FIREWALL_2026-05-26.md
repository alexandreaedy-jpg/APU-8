# V2B Step5Y DMC Firewall

Date: 2026-05-26

## Pourquoi

Le log `STEP2B-Panel_Control_test13` montre deux problemes distincts:

- le transport peut saturer avant meme que le DMC soit servi (`OVF=1`, `N=.../32`, `DMC=--/0`)
- la ROM pouvait encore classer une valeur `AUX` comme DMC avant de respecter `STATUS_AUX_IS_TRI`

Donc le bug entendu comme "DMC sur TRI" n'etait pas seulement une question de sample: un meta DMC en attente pouvait encore rendre le decodeur trop permissif.

## Changements

- Pico `FW=t54-v2b-step5y-dmcfirewall`
- ROM: `STATUS_AUX_IS_TRI` gagne toujours contre les signatures DMC `FD/FE/E*`
- ROM: si un paquet non-DMC arrive pendant qu'un meta DMC est arme, l'armement DMC est annule
- Pico: DMC ne s'enfile plus si une voix musicale ou un controle est deja pending
- Pico: DMC queue limitee a 2 octets pour eviter les vieux samples retardes
- Pico: controles panel injectes seulement quand la queue musicale est vide
- Pico: cadence controle panel durcie a une paire `meta/value` toutes les `16 ms`
- Pico: queues voix montees a 48 slots pour absorber les rafales sans `OVF` immediat

## Resultat attendu

- aucune valeur `TRI` ne doit pouvoir declencher un sample DMC
- si le systeme est charge, le DMC droppe (`DD++`) plutot que de casser le groove
- les controles peuvent devenir moins instantanes sous charge, mais ne doivent plus voler des slots aux notes
- les decrochages restants doivent etre des limites de debit musical, pas des substitutions d'instrument

## Points a surveiller

- Si `DD` monte, c'est volontaire: le DMC est sacrifie pour proteger `P1/P2/TRI/NOISE`
- Si `OVF` monte sans DMC et sans mouvement panel, la limite est le debit note/arp/noise lui-meme
- Si les controles semblent lents pendant un arp dense, c'est le nouveau comportement de securite: latest-value-wins, injection seulement en trou de trafic
