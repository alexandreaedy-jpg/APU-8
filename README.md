# APU-8

APU-8 is an experimental MIDI-controlled NES instrument project.

The goal is to turn a real NES / Famicom into a playable external sound module with:

- Pulse 1
- Pulse 2
- Triangle
- Noise
- Samples

using custom ROM code, external control hardware, and eventually a more robust bus interface than the standard controller port.

## Current status

This repository contains both:

- the **working history** of the controller-port generations (`V15` to `V18`)
- the **research and planning** for a future **V2 expansion-port architecture**

The current situation is roughly:

- `V16` proved that a split controller-port design could work
- `V17` gave the best usable vibrato result by treating modulation as **preset/config** instead of a continuous live stream
- `V18` is focused on transport precision and controller-port limits
- `V2` is using the **NES-001 bottom expansion port**

This is an active hardware/software research project, not a finished product.

## Main idea

The first major architecture used the NES controller port as a custom multi-lane transport:

- `D0` = notes / gate / triggers
- `D3` = ADSR + slower controls
- `D4` = LFO / modulation config

The best result so far came from a simple lesson:

**slow musical configuration is much more reliable than trying to stream too much live modulation state through a constrained transport.**

That lesson is now shaping both:

- the late `V17 / V18` controller-port work
- the planned `V2` expansion-port design

## Repository structure

```text
APU-8/
├── arduino/                  # Arduino / Nano sketches
│   ├── NanoNesV17DN4/
│   ├── NanoNesV18DN4/
│   └── ...
├── project-v17-DN4/          # V17 ROM branch
├── project-v18-DN4/          # Current ROM/research branch
├── project-v16-DN3/          # Earlier stable milestone
├── milestones/               # Frozen milestone snapshots
├── max-for-live/             # Ableton / control experiments
├── cc65-2.13.3/              # 6502 toolchain
├── bridge.py                 # Host-side bridge experiments
├── bridge_separate_channels.py
└── README.md
```

## Important documents

### Current architecture / planning

- [project-v18-DN4/V18_1_ACTION_PLAN.md](project-v18-DN4/V18_1_ACTION_PLAN.md)
- [project-v18-DN4/LFO_V2_ARCHITECTURE.md](project-v18-DN4/LFO_V2_ARCHITECTURE.md)
- [project-v18-DN4/V18_CLK_PROTOCOL.md](project-v18-DN4/V18_CLK_PROTOCOL.md)

### V2 expansion-port work

- [project-v18-DN4/V2_EXPANSION_PORT_BOM_2026-05-04.md](project-v18-DN4/V2_EXPANSION_PORT_BOM_2026-05-04.md)
- [project-v18-DN4/V2_EXPANSION_PORT_PIN_PLAN.md](project-v18-DN4/V2_EXPANSION_PORT_PIN_PLAN.md)
- [project-v18-DN4/V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04.md](project-v18-DN4/V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04.md)
- [project-v18-DN4/V2_GPIO_BUDGET_AND_CONTROLS.md](project-v18-DN4/V2_GPIO_BUDGET_AND_CONTROLS.md)

### Research notes

- [project-v18-DN4/FAMIMIMIDI_RESEARCH_NOTES.md](project-v18-DN4/FAMIMIMIDI_RESEARCH_NOTES.md)

## Versions at a glance

### V15

- very early `D0`-focused transport work

### V16

- important stable milestone
- `D0` notes stable
- `D3` ADSR + duty stable
- `D4` LFO stable

### V17

- cleaner modulation direction
- vibrato presets proved much more viable than continuous live modulation streaming

### V18

- transport redesign
- focus on note precision, trigger timing, and controller-port limits

### V2

- flashcart still used for ROM/runtime
- external hardware moved toward the NES-001 bottom expansion port
- RP2040-based external interface

## Hardware overview

### Controller-port generations

Typical V1.x setup:

- NES / Famicom
- flashcart / EverDrive
- Arduino Nano
- MIDI DIN input
- custom wiring to controller-port data lines

### V2 direction

Current V2 planning targets these expansion-port signals:

- `CPU D0..D7`
- `A15`
- `OUT0..OUT2`
- `/IRQ`
- `+5V`
- `GND`

Important caveat:

- the NES-001 bottom expansion port does **not** expose CPU `R/W`

so V2 needs a custom handshake/latch strategy, not a generic memory-mapped design.

## Toolchain

This repository uses `cc65` for NES ROM builds.

The ROM work is primarily inside:

- [project-v17-DN4](project-v17-DN4)
- [project-v18-DN4](project-v18-DN4)

Each project directory contains its own `build.ps1` and ROM sources.

## Why this project exists

Many NES MIDI projects already exist, but they tend to fall into different tradeoffs:

- simple but limited
- powerful but tightly specialized
- strongly dependent on PC-side software

APU-8 is an attempt to find a balance between:

- real hardware feel
- direct musical control
- robust note timing
- and an architecture that can grow without collapsing under its own transport complexity

## Known limitations

At the current stage:

- controller-port transport is still a major source of constraints
- modulation quality depends heavily on how much logic is kept out of the urgent note path
- some branches are exploratory and not all are musically stable
- V2 is planned, but not yet implemented

## References

The project has been informed in part by public work and documentation around:

- ChipMaestro
- MIDINES
- NESizer2
- Famimimidi
- SynthNes

These references are documented more explicitly in the research notes inside `project-v18-DN4`.

## Feedback welcome

If you have experience with:

- NES controller-port timing
- custom expansion-port hardware
- MIDI-to-APU workflows
- cart-side synth runtimes

feedback is very welcome.

This project is still evolving, and outside technical input is extremely valuable at this stage.
