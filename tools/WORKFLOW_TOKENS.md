# Workflow economique en tokens

Objectif: reduire les messages a des commandes courtes et reutilisables.

## Commandes locales

Depuis `C:\Users\mto1\Documents\NES_DEV`:

```powershell
.\tools\v2.ps1 status midi
.\tools\v2.ps1 build-rom midi
.\tools\v2.ps1 push-rom midi
.\tools\v2.ps1 build-pico midi
.\tools\v2.ps1 flash-pico midi
.\tools\v2.ps1 deploy midi
```

Version encore plus courte recommandee:

```powershell
.\tools\v2.cmd status midi
.\tools\v2.cmd build-rom midi
.\tools\v2.cmd push-rom midi
.\tools\v2.cmd build-pico midi
.\tools\v2.cmd flash-pico midi
.\tools\v2.cmd deploy midi
```

Raccourcis equivalents:

```powershell
.\tools\build-midi.cmd
.\tools\push-midi-rom.cmd
.\tools\flash-midi.cmd
.\tools\deploy-midi.cmd
```

`deploy-midi.ps1` reconstruit la ROM, pousse `D:\game.nes`, compile le Pico, puis flashe `COM6`.

## Vocabulaire court pour les prochaines iterations

Tu peux demander:

- `build midi`: build ROM + Pico, sans flash.
- `rom midi`: pousse seulement la ROM vers `D:\game.nes`.
- `flash midi`: flashe seulement le Pico.
- `deploy midi`: ROM + Pico complet.
- `log midi`: analyse le dernier log MIDI fourni.
- `patch midi compact`: corrige uniquement la variante MIDI, sans toucher aux steps valides.

## Discipline projet

- Toujours nommer la cible: `midi`, `step14`, `step25`, etc.
- Un message = une intention courte.
- Quand un test echoue, fournir seulement: symptome auditif + log si disponible.
- Les steps valides restent intouchables sauf demande explicite.
- Les variantes experimentales vivent dans un dossier dedie, pas dans les bases validees.

## Criteres rapides

- Silence sans MIDI.
- Canal 12 = P1.
- Canal 13 = P2.
- `Q` bas.
- `OVF=0`.
