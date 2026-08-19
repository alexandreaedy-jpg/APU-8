## Suggested thread title

APU-8: NES MIDI instrument experiment, controller-port limits, expansion-port V2?

## Short draft post

Hello,

I have been building a real-NES MIDI instrument project that currently uses:

- a custom ROM on flashcart / EverDrive
- an Arduino Nano for MIDI + control handling
- the NES controller port as a custom transport

The goal is a playable external NES sound module with stable note timing, usable controls, and modulation that does not fall apart under load.

My controller-port design evolved into:

- `D0` = notes / gate / triggers
- `D3` = ADSR + slower controls
- `D4` = modulation / vibrato config

This actually worked to a point. I had a stable version where:

- notes were stable
- ADSR + duty were stable
- vibrato presets were usable

The best result so far came when `D4` stopped behaving like a live modulation stream and became a **slow config lane**, with the ROM owning the actual modulation runtime.

The remaining problem is that the controller port still seems too fragile for the kind of instrument I want. As I pushed for better timing, I ran into:

- lost notes
- cross-triggering
- envelope corruption
- modulation being perturbed by note traffic

So I am now considering a **V2** based on the **NES-001 bottom expansion port**, while still keeping a flashcart in the cartridge slot.

The signals I am currently interested in are:

- `CPU D0..D7`
- `A15`
- `OUT0..OUT2`
- `/IRQ`

One important caveat is that the bottom expansion port does **not** expose CPU `R/W`, so this would need some kind of custom handshake / latch design rather than a normal memory-mapped peripheral approach.

What I would love feedback on:

1. Has anyone here built custom hardware on the NES-001 bottom expansion port while still running code from a flashcart or cartridge?
2. What would be the cleanest protocol strategy using the exposed signals, given that `R/W` is not available there?
3. Has anyone had success using controller-port shift-register style transport for non-controller musical data under real load?
4. If anyone knows of public technical info on projects like Famimimidi, I would be very interested.

I can share code / wiring / more detailed notes if useful. I am cleaning the project up for GitHub now.
