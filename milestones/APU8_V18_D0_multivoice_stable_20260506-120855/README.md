# APU8 V18 D0 Multivoice Stable

Snapshot stable valide le `2026-05-06`.

Etat valide:
- Nano flashe avec `arduino/NanoNesV18DN4_multivoice/build-compact-timing`
- ROM multivoix compacte `project-v18-DN4_multivoice/game.nes`
- transport `D0` compact a `9` octets
- parser ROM incremental: `1` octet lu par poll, plus de lecture bloquante de trame complete
- cablage valide:
  - `NES OUT/LATCH -> Nano D12`
  - aucun `NES CLK -> Nano D13`
  - `CLK` et `LATCH` restent branches vers le `4021`
- stable au test utilisateur, y compris avec `5` voix simultanees

Hashes:
- `NanoNesV18DN4_multivoice_build-compact-timing.hex`
  - `27DC80F4CD87245CAC215277A595CFF6754FF5C369B9B4D332CCA433916BC12C`
- `project-v18-DN4_multivoice_game.nes`
  - `121806A140578FE6C5AB42D297E1BE007CCB0366B126E5ECA81E2F2F5F2188DF`

Contenu:
- `NanoNesV18DN4_multivoice_build-compact-timing.hex`
- `NanoNesV18DN4_multivoice_build-compact-timing.elf`
- `project-v18-DN4_multivoice_game.nes`
- `NanoNesV18DN4_multivoice.ino`
- `project-v18-DN4_multivoice_main.c`
- `MULTIVOICE_PROTOCOL_NOTES.md`

Restauration:
- lancer `restore_v18_d0_multivoice_stable.ps1`
- ou flasher le Nano avec le `.hex` du snapshot puis copier `project-v18-DN4_multivoice_game.nes` vers `D:\game.nes`

Notes:
- la base `P1` stable precedente reste figee dans `milestones/APU8_V18_D0_P1_stable_20260506-040940`
- ce snapshot est la nouvelle base de travail pour le multi-voix `D0`
