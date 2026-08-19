# V2B Step5H Physical Panel MCP3008 - 2026-05-20

## But

Ajouter les controles physiques au profil stable `Step5G`, sans casser le transport MIDI valide.

Principe:
- le panel pilote les memes etats internes que les CC MIDI;
- le Pico garde l'etat en memoire;
- un controle stable ne genere aucun trafic NES;
- seule une valeur quantifiee differente est envoyee.

## Revision

- Pico firmware: `t37-v2b-step5h-panel`
- ROM: protocole/audio inchanges par rapport a Step5G, rebuild seulement.
- Heartbeat ajoute: `PC=` pour compter les actions appliquees depuis le panel.

## Cablage suppose

### MCP3008 vers Pico

- `MCP3008 DOUT` -> `GP16` (`MISO`)
- `MCP3008 CS/SHDN` -> `GP17`
- `MCP3008 CLK` -> `GP18`
- `MCP3008 DIN` -> `GP19` (`MOSI`)
- `VDD/VREF` -> `3.3V`
- `AGND/DGND` -> `GND`

### MCP3008 canaux

- `CH0`: Volume
- `CH1`: Attack
- `CH2`: Decay
- `CH3`: Release
- `CH4`: LFO Depth
- `CH5`: LFO Rate
- `CH6`: Duty / Noise Timbre
- `CH7`: Arp Time

### Switches deja branches

Actifs a `LOW` avec `INPUT_PULLUP`:
- `GP20`: cible `P1`
- `GP21`: cible `P2`
- `GP22`: cible `TRI`
- `GP26`: cible `NOISE`
- `GP27`: cible `GLOBAL`
- `GP28`: arp on/off
- `GP8`: LFO cible pitch
- `GP9`: LFO cible duty
- `GP15`: LFO cible amp

## Comportement

### V/A/D/R

Envoyes a la voix selectionnee.

En `GLOBAL`:
- `NOISE` d'abord;
- puis `P1`;
- puis `P2`;
- `TRI` reste volontairement hors enveloppe.

### LFO Depth

Le potard `LFO Depth` depend du selecteur `LFO target`:
- `GP8`: pitch LFO
- `GP9`: duty LFO
- `GP15`: amp LFO

Le code respecte les garde-fous Step5:
- pas de LFO non-zero si l'arp de la voix est actif;
- pas de LFO `TRI` dans ce profil de stabilite;
- `NOISE` accepte surtout amp/rate selon les controles deja autorises.

### LFO Rate

Applique a la cible voix selectionnee, ou a `NOISE/P1/P2` en global.

### Duty / Timbre

- `P1/P2`: 3 positions duty utiles.
- `NOISE`: timbre 4 bits Step5F.
- `GLOBAL`: `NOISE` recoit le timbre 4 bits; `P1/P2` recoivent leur duty 3 positions.

### Arp Time

4 divisions:
- zone 0: division 0
- zone 1: division 1
- zone 2: division 2
- zone 3: division 3

Le code ignore les repetitions dans la meme zone.

### Arp On/Off

`GP28` applique l'arp on/off a la cible selectionnee.

En `GLOBAL`, applique seulement aux voix qui supportent l'arp:
- `P1`
- `P2`
- `TRI`

`NOISE` reste sans arp.

## Filtrage

- scan panel toutes les `12 ms`;
- filtrage simple des lectures MCP;
- deadband raw leger;
- latch quantifie par canal;
- changement de selecteur force une relecture de l'etat courant.

## Test recommande

1. Flasher le Pico `t37-v2b-step5h-panel`.
2. Verifier que le heartbeat affiche `PC=`.
3. Selectionner `P1`, bouger `V/A/D/R`, `Duty`, `LFO`.
4. Selectionner `NOISE`, tester `Release` et `Duty/Timbre`.
5. Tester `GLOBAL`.
6. Verifier que `PC=` ne monte pas quand les controles ne bougent pas.

Si `NOISE` devient reactif en panel mais pas via Live, Live/CC est le coupable.
Si `NOISE` reste mou en panel solo, il faudra revoir le mapping audio du timbre/release noise.
