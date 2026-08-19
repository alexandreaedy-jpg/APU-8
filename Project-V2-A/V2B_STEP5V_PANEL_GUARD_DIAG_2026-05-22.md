# V2B Step5V Panel Guard + CH0 Diagnostic - 2026-05-22

## Objectif

Step5V garde la base Step5U et traite deux problèmes séparément:

- diagnostiquer `Slider1` / `MCP3008 CH0` sans bruit MIDI/NES;
- protéger le groove quand le panel bouge sous charge.

## Diagnostic MCP3008

Nouveau sketch isolé:

- `arduino\PicoMcp3008RawDiag\PicoMcp3008RawDiag.ino`
- `DOUT=GP16`, `CS=GP17`, `CLK=GP18`, `DIN=GP19`
- sortie série toutes les `100 ms`: `RAW`, `MIN`, `MAX`, `MOVE`

Tests à faire:

- Slider1 sur `CH0`: `RAW0` doit balayer une grande plage.
- Slider1 sur `CH1`: si `RAW1` bouge bien, le slider/wiper est OK.
- Contrôle connu OK sur `CH0`: si `RAW0` reste bloqué, suspecter pin `CH0`, soudure, piste ou MCP3008.
- `CH0` forcé à `GND`, puis `3.3V`: attendu proche de `0`, puis proche de `1023`.

## Protection Panel Step5V

- Firmware principal: `FW=t51-v2b-step5v-panelguard`.
- Le transport note reste immédiat.
- Les contrôles restent coalescés en `latest value wins`.
- Le scheduler n'injecte une paire `meta/value` que si `Q<=2`.
- Les contrôles sont cadencés à une paire maximum toutes les `6 ms`.
- `Volume=0` venant du panel est clampé à `1` tant que `CH0` n'est pas fiable.
- `CH7` en mode `Arp Time` ou `LFO Delay` doit rester stable `40 ms` avant envoi.
- Le log raw panel compact `PR=` existe mais reste désactivé par défaut (`ENABLE_PANEL_RAW_LOG=false`).

## Validation

- En diagnostic brut, identifier si la panne suit le slider, le fil, `CH0`, ou le MCP3008.
- En live, les mouvements panel ne doivent plus provoquer de passages muets.
- En stress, `OVF=0` doit rester vrai, et `Q` doit redescendre rapidement après les rafales.
- Si le groove décroche encore panel immobile, tester en note-only avant de conclure à une limite du bus expansion.
