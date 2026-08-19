# APU8 V18 D0 P1 Stable

Snapshot stable valide le `2026-05-06`.

Etat valide:
- Nano flashé avec `build-final-normal`
- ROM normale `project-v18-DN4/game.nes`
- proto `D0-only`
- `D12` et `D13` débranchés du Nano
- `LATCH` et `CLK` restent branchés vers le `4021`
- `P1` sain et réactif

Hashes:
- `NanoNesV18DN4_build-final-normal.hex`
  - `8A702B5EDF297AB0046F89539F9EADC53BC80FECF67C35B1B35491D584022856`
- `project-v18-DN4_game.nes`
  - `A4CCCB68FF1BD868F35D5ED8B0617866B263573E5307DD06B7B88FFE3FCF8F5E`

Contenu:
- `NanoNesV18DN4_build-final-normal.hex`
- `NanoNesV18DN4_build-final-normal.elf`
- `project-v18-DN4_game.nes`

Restauration:
- flasher le Nano avec le `.hex` stable
- copier `project-v18-DN4_game.nes` vers `D:\game.nes`

Note importante:
- le source courant `arduino/NanoNesV18DN4/NanoNesV18DN4.ino` a été réaligné pour recompiler vers le même `.hex` que `build-final-normal`
- les essais multi-voix doivent repartir d'une copie de travail séparée, pas de cette base stable
