## APU-8 V2B KiCad bundle

Ce dossier contient une base exploitable pour reconstruire proprement le schema dans KiCad a partir de l'etat gele du projet.

Contenu :

- `APU8_V2B_kicad_netlist.xml`
  - netlist XML de style KiCad/Eeschema
  - utile comme source de verite de cablage
  - volontairement fonctionnel plutot que footprint-precis pour eviter d'inventer un pinout faux
- `APU8_V2B_pinmap.csv`
  - table de cablage lisible et importable dans un tableur
- `APU8_V2B_design_notes.md`
  - notes de conception, points figes, lignes optionnelles/debug

Ce bundle decrit la base "figee" actuelle :

- NES expansion port comme interface principale
- RP2040 Pico pour sniff/control
- `74HC4050` en entree NES -> Pico, alimente en `3.3V`
- `74HCT245` en sortie Pico -> NES, alimente en `5V`
- `MCP3008` pour les 8 entrees analogiques du panel

Important :

- `A15` et `CPU D0..D2` sont conserves ici comme **optionnels/debug**
- le transport audio stable n'en depend pas
- si `A15` n'est pas cable sur le premier `74HC4050`, ce buffer utilise `4 / 6` canaux, donc il reste **2 canaux libres**

Sources consolidees :

- `C:\Users\mto1\Documents\NES_DEV\ROM V2B\V2_EXPANSION_PORT_PIN_PLAN.md`
- `C:\Users\mto1\Documents\NES_DEV\ROM V2B\V2_PROTO1_BREADBOARD_ARCHITECTURE.md`
- `C:\Users\mto1\Documents\NES_DEV\Firmware Arduino\Firmware_PICO-V2B\Firmware_PICO-V2B.ino`
- `C:\Users\mto1\Documents\NES_DEV\PROJECT_STATE.md`
