## Commit-Pending Experiment

Base:
- sketch derive du moteur `D3` physique en cours
- rollback stable conserve via `C:\Users\mto1\Documents\NES_DEV\restore_v18_d0_multivoice_stable.ps1`

But:
- sortir le prechargement `74HC595` du chemin critique `LATCH`
- laisser l'ISR poser seulement un `commit_pending`
- executer `shiftD3AdvanceFrame()` puis `shiftD0AdvanceFrame()` dans `loop()`

Etat actuel:
- fichier de travail: [Project-V2-A-Nano-commitpending.ino](C:/Users/mto1/Documents/NES_DEV/arduino/Project-V2-A-Nano-commitpending/Project-V2-A-Nano-commitpending.ino)
- build compile localement
- aucun flash de cette variante n'a encore ete impose a la base stable

Rollback:
1. lancer `restore_v18_d0_multivoice_stable.ps1`
2. verifier `D:\game.nes`
3. retester le snapshot stable
