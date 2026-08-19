# Build Toolchain Status

Ce document resume l'etat reel de la chaine de build NES dans ce workspace.

## Diagnostic

Le projet NES utilise la librairie et les fichiers de configuration issus des exemples Shiru / neslib.

Le fichier [project/readme.txt](/C:/Users/mto1/Documents/NES_DEV/project/readme.txt) indique explicitement :

- compilation attendue avec `CC65 v2.13.3`
- les versions plus recentes demandent des adaptations de linker
- la librairie n'a jamais ete migree officiellement

## Etat Local Confirme

Compilateurs disponibles localement :

- `cc65-snapshot-win32/bin/cc65.exe` -> `cc65 V2.19 - Git 80ff9d3`
- `cc65-snapshot-win64/bin/cc65.exe` -> `cc65 V2.19 - Git 80ff9d3`

Fichiers projet verifies :

- [project/runtime.lib](/C:/Users/mto1/Documents/NES_DEV/project/runtime.lib)
- [cc65_nes_examples/runtime.lib](/C:/Users/mto1/Documents/NES_DEV/cc65_nes_examples/runtime.lib)
- [project/crt0.s](/C:/Users/mto1/Documents/NES_DEV/project/crt0.s)
- [project/neslib.h](/C:/Users/mto1/Documents/NES_DEV/project/neslib.h)

Constat important :

- `project/runtime.lib` et `cc65_nes_examples/runtime.lib` sont identiques
- les outils `V2.19` savent compiler `main.c`
- mais ils ne sont pas compatibles de facon fiable avec cette ancienne stack complete
- resultat : on peut obtenir une ROM qui linke, mais qui ne reste pas musicalement fiable

## ROM Stable De Reference

La base binaire stable actuelle est :

- [project-clean/game-test.nes](/C:/Users/mto1/Documents/NES_DEV/project-clean/game-test.nes)

Cette ROM sert de reference stable tant que la vraie toolchain `CC65 v2.13.3` n'est pas reconstituee localement.

La ROM de travail peut etre restauree a partir de cette base.

## Regle Pratique

Tant que `CC65 v2.13.3` n'est pas disponible localement :

- ne pas considerer un rebuild `V2.19` comme fiable
- ne pas remplacer la ROM stable sans validation immediate dans Mesen
- garder [project-clean/game-test.nes](/C:/Users/mto1/Documents/NES_DEV/project-clean/game-test.nes) comme point de retour

## Strategie Recommandee

1. retrouver / installer `CC65 v2.13.3`
2. recreer un script de build strict avec cette version
3. verifier qu'une recompilation propre produit une ROM audio stable
4. seulement ensuite reprendre les gros changements ROM

## Ce Qui Est Securise Maintenant

- le diagnostic est documente
- la ROM stable de reference est identifiee
- un script de restauration est fourni dans `project-clean`

## Ce Qui Reste A Faire

- obtenir la vraie toolchain `CC65 v2.13.3`
- ecrire le script de build final autour de cette version
