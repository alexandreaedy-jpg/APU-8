# APU-8

Système MIDI avancé pour Nintendo Entertainment System (NES) avec contrôle en temps réel de l'APU (Audio Processing Unit) via port parallèle.

## 🎵 Caractéristiques

- **4 canaux indépendants** : Pulse 1, Pulse 2, Triangle, Noise
- **ADSR complet** : Attack, Decay, Sustain, Release par voix
- **LFO V2** : Tremolo et vibrato (en développement)
- **Contrôle en temps réel** : Communication parallèle D0/D3/D4
- **Arduino Nano** : Interface MIDI vers NES

## 🏗️ Architecture

### Ports de données
- **D0** : Notes ON/OFF, vélocité
- **D3** : Enveloppes ADSR, duty cycle
- **D4** : Configuration LFO V2

### Matériel requis
- NES/Famicom avec port d'extension modifié
- Cartouche EverDrive ou flashable
- Arduino Nano (contrôleur MIDI)
- Interface parallèle (74HC595/74LS373)

## 📁 Structure du projet

```
NES_DEV/
├── arduino/           # Sketch Arduino Nano
│   └── NanoNesV17DN4/
├── project-v18-DN4/   # Version actuelle (ROM + sources)
│   ├── main.c         # Code source principal
│   ├── game.nes       # ROM compilée
│   └── *.md           # Documentation
├── cc65-2.13.3/       # Toolchain de compilation
└── max-for-live/      # Devices Ableton Max for Live
```

## 🚀 Compilation

### Prérequis
- cc65 (toolchain 6502)
- ca65, ld65
- Make ou PowerShell

### Build ROM
```powershell
cd project-v18-DN4
..\cc65-2.13.3\cc65\bin\cc65 -Oi main.c --add-source
..\cc65-2.13.3\cc65\bin\ca65 main.s
..\cc65-2.13.3\cc65\bin\ld65 -C nrom_256_vert.cfg -o game.nes crt0.o main.o dmc_samples.o runtime.lib
```

## 🔌 Câblage

### Vers NES (Port d'extension)
```
Arduino D2  → NES D0 (Data 0 - Notes)
Arduino D3  → NES D3 (Data 3 - ADSR)
Arduino D4  → NES D4 (Data 4 - LFO)
Arduino D5  → NES CLK (Clock)
```

### Contrôleurs analogiques
```
A0 = Attack
A1 = Decay
A2 = Sustain
A3 = Release
A4 = Duty
A5 = Rate (LFO)
A6 = Depth (LFO)
```

## 📚 Documentation

- `CONTROL_PROTOCOL.md` : Protocole de communication D0/D3/D4
- `LFO_V2_ARCHITECTURE.md` : Architecture LFO V2
- `V18_1_PROTO_WIRING.md` : Schémas de câblage V18

## 🎯 Versions

- **v15** : D0 seulement (notes de base)
- **v16-DN3** : D0 + D3 (ADSR)
- **v17-DN4** : D0 + D3 + D4 (LFO V2)
- **v18-DN4** : Version actuelle (optimisée)

## ⚠️ Limitations connues

- LFO V2 : Le tremolo n'est pas encore parfaitement sinusoïdal
- Pitch bend : Non implémenté en hardware
- Triangle : LFO non supporté (contraintes hardware 6502)

## 📜 Licence

Projet open source. Inspiré de :
- ChipMaestro (Stanislavche)
- FamiMIDI (Captain)
- NESizer2 (Jaffe)

---
Développé par Alexandreaedy
