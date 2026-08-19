# V2B Step5R - Pot Arp Time / LFO Delay

Revision Pico: `t47-v2b-step5r-arpdelay`

Objectif: ne pas laisser le potentiometre `Arp Time` inactif quand l'arp est desactive.

## Comportement

- `MCP3008 CH7` reste `Arp Time` quand le switch arp physique est ON.
- `MCP3008 CH7` devient `LFO Delay` quand le switch arp physique est OFF.
- Le basculement ON/OFF applique immediatement la valeur courante du potentiometre.
- Le mode `Arp Time` garde 4 divisions internes.
- Le mode `LFO Delay` utilise toute la resolution 4-bit `0..15`.

## Notes

- Ce changement ne modifie pas le transport NES ni la ROM.
- En heartbeat, `PV` continue d'afficher la valeur quantifiee du canal 7.
- `PA=AR:x` indique une division arp appliquee.
- `PA=DL:x` indique un delay LFO applique.
