# V2B Step5AL - Linear Volume Wave Release

Date: 2026-05-27

Base: Step5AK `midi-v2b15` / `FW=t66-v2b-step5ak-volcurve`.

## Diagnostic

- La courbe Step5AK de volume etait trop concentree en bas de course.
- Besoin utilisateur: relation directe potard/volume, ex. 10% = 10%, 50% = 50%.
- Release max encore un peu court.
- Le controle LFO delay doit etre remplace par waveform.

## Changements

- Nouveau firmware: `FW=t67-v2b-step5al-linvolwave`.
- Volume panel lineaire via `panelAnalogToNibble()`.
- ROM: release rallonge par rapport a Step5AK.
- Panel:
  - `MCP3008 CH7` = `Arp Time` si ARP ON.
  - `MCP3008 CH7` = `LFO Wave` si ARP OFF.
- Ordre waveform: `Sine > Tri > Saw > Square`.
- MIDI:
  - `CC34` devient un alias waveform au lieu de delay.
  - `CC35` reste waveform, meme ordre.

## A verifier

- Log: `FW=t67-v2b-step5al-linvolwave`.
- `PV=`: le deuxieme chiffre doit monter lineairement avec le potard volume.
- Release max: doit etre un peu plus long que Step5AK.
- Avec ARP OFF, le potard CH7 doit changer la waveform LFO dans l'ordre `Sine/Tri/Saw/Square`.
- Avec ARP ON, CH7 doit encore controler l'arp time.
