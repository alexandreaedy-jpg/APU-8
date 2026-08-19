# V2B Step5AJ - Poly Delay Release Anti-Pop

Date: 2026-05-27

Base: Step5AI `midi-v2b13` / `FW=t64-v2b-step5ai-fxtri`.

## Diagnostic

- Step5AI est valide cote effets, mais le release maximum est environ 50% trop court.
- Le controle MIDI `CC34 LFO delay` etait encore filtre par le mode panel-debug.
- Le canal `CH16` servait surtout aux controles globaux; les notes globales/poly n'etaient pas encore distribuees vers `P1/P2/TRI`.
- Les pops pendant le glide sont coherents avec le comportement documente de l'APU NES: ecrire `$4003/$4007` pendant une note reset la phase/enveloppe du pulse.

## Changements

- ROM: release rallonge d'environ 50% par rapport a Step5AI.
- ROM: duck tres court de volume avant les ecritures mid-note `$4003/$4007`.
- Pico: `FW=t65-v2b-step5aj-polydelay`.
- Pico: `CC34 LFO delay` est reactive.
- Pico: le panel global inclut maintenant `TRI` pour les controles LFO compatibles.
- Pico: `CH16` note-on/note-off devient polyphonique sur `P1/P2/TRI`.
- Pico: allocation poly simple:
  - slot libre d'abord;
  - sinon vol de la voix la plus ancienne;
  - le note-off relache la voix assignee.

## A verifier

- Log: `FW=t65-v2b-step5aj-polydelay`.
- Release max: doit etre plus long que Step5AI, sans revenir a l'ancien release trop long.
- `CC34` doit retarder l'entree du LFO.
- Panel `Arp Time/LFO Delay` avec ARP OFF doit encore controler le delay.
- `CH16` notes: accords 3 notes sur `P1/P2/TRI`.
- Tester un glide pulse large: les pops doivent etre reduits, mais pas forcement totalement elimines si le high timer traverse plusieurs fois.
- `OVF` doit rester a `0`.
