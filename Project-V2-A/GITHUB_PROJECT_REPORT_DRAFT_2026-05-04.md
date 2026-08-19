## APU-8 Project Report Draft

This document is a long-form project summary intended for a GitHub repository page or project update post.

It is written to explain:

- what the project is
- how the architecture evolved
- what worked
- what failed
- what was learned
- where the project is likely going next

---

## 1. Project overview

APU-8 is an attempt to turn a real NES into a playable external MIDI-controlled instrument.

The main target is not only basic MIDI triggering, but a musical workflow with:

- direct access to the 2A03 APU voices
- meaningful per-voice control
- stable modulation
- low-latency note response
- hardware controls where possible

The voices targeted are:

- Pulse 1
- Pulse 2
- Triangle
- Noise

DPCM and related extensions are also part of the long-term scope, but note timing and control architecture were always the first priorities.

---

## 2. Early architecture

The early development path was based on:

- a custom NES ROM running from a flashcart / EverDrive
- an external Arduino Nano
- MIDI DIN input
- a hardware control surface
- custom transport over the NES controller port

The project moved away from a global voice allocator and toward a direct multi-lane mapping:

- `P1`
- `P2`
- `TRI`
- `NOISE`

This was a deliberate attempt to make the system feel more like a dedicated synth and less like a generic MIDI-to-NES mapper.

---

## 3. The separate-lane controller-port model

The main controller-port design eventually became:

- `D0` = notes / gate / trigger
- `D3` = ADSR + duty / slower control data
- `D4` = modulation / vibrato / config

The idea behind that split was simple:

- keep urgent note traffic away from control traffic
- stop large control packets from interfering with the most timing-sensitive path
- let the ROM own more of the musical runtime locally

This concept produced some encouraging results, but also exposed the hard limits of the controller port.

---

## 4. What actually worked

### V16: stable `D0 + D3 + D4`

One key milestone was a stable **V16** state where:

- `D0` notes were stable
- `D3` ADSR + duty were stable
- `D4` LFO was stable
- selector behavior was stable
- ROM and Nano were aligned

This proved that the multi-lane idea was viable in principle.

### V17: better LFO behavior through presets

The next major insight was that `D4` worked much better as a **slow configuration lane** than as a continuously changing live modulation stream.

That led to a much more successful vibrato strategy:

- quantized presets
- ROM-owned oscillator behavior
- less continuous traffic on `D4`

This was one of the first moments where the project felt musically convincing rather than just technically interesting.

---

## 5. What kept failing

### 5.1 Controller-port transport fragility

As the design was pushed harder, especially around note precision and modulation cleanliness, the controller port started showing its limits:

- random note loss
- cross-triggering between voices
- envelope corruption
- unstable reads
- some builds producing no sound at all

The symptoms were consistent with bit/frame desynchronization and overloading the transport model.

### 5.2 LFO pollution from note traffic

Even after `D4` was cleaned up, note traffic on other lanes could still perturb the audible modulation result.

This suggested that the issue was no longer just `D4` itself, but the fact that:

- transport
- event handling
- envelope logic
- modulation logic
- APU writes

were still too entangled at runtime.

### 5.3 Mechanical reliability of the controller-port hardware

Another painful lesson was that physical connection quality matters as much as the protocol.

At several points, unstable or incorrectly reconstructed controller-port cabling likely polluted the debugging process. This made it clear that a marginal connector can make software conclusions meaningless.

---

## 6. Lessons from existing projects

Several existing projects were reviewed during development:

- MIDINES
- ChipMaestro
- NESizer2
- Famimimidi
- SynthNes

### MIDINES

Useful as a reference point, but clearly limited compared with newer or more specialized approaches.

### ChipMaestro

The most useful lesson was not "copy this exact hardware", but:

- short targeted transfers
- less runtime protocol cleverness
- less pressure on the software at the timing-critical instant

### NESizer2

The most important lesson here was the separation of timing domains:

- input handling
- modulation update
- envelope update
- parameter application
- APU refresh

That helped clarify why some APU-8 modulation experiments kept collapsing under note traffic.

### Famimimidi

The public docs suggest a powerful cart-side synth runtime with:

- direct APU-facing behavior
- lots of built-in musical control
- no dependence on a normal on-screen UI

This reinforced the idea that stronger results come from local synth ownership near the bus, not from trying to stream too much live state through a constrained interface.

### SynthNes

Interesting technically, but less attractive as an instrument because of the stronger dependence on a Windows/USB workflow centered around a flashcart.

---

## 7. The V18 / V18.1 rethink

By this point, the project had reached a new conclusion:

the controller port should not be treated like a rich custom packet bus.

That led to the current V18.1 design direction:

- `D0` becomes the only urgent lane
- `D3` and `D4` move fully outside the critical note path
- the transport should behave like a proper controller / shift-register stream
- the ROM should own the musical runtime

The corresponding action plan was formalized in:

- [V18_1_ACTION_PLAN.md](C:/Users/mto1/Documents/NES_DEV/project-v18-DN4/V18_1_ACTION_PLAN.md)

This narrowed the problem in a healthy way:

- first make note transport sane
- only then restore slower control layers

---

## 8. Hardware-assisted controller-port experiments

To reduce timing pressure on the microcontroller, a hardware-shift experiment was started using:

- `74HC595`
- `4021`

The idea was:

- the microcontroller prepares the byte
- the 595 holds it in parallel
- the 4021 behaves like a controller-side shift register
- the NES clocks the bits itself

Conceptually, this is much cleaner than asking the Nano to improvise every critical bit transition in software.

Even if this branch does not become the final production architecture, it already provided an important lesson:

moving the timing-critical path into simple hardware logic is often better than trying to outsmart the NES with protocol tricks.

---

## 9. Why a V2 is now being planned

As the project matured, it became harder to ignore that the controller port may simply not be the best long-term transport for a serious instrument.

This led to planning for a **V2** using the **NES-001 bottom expansion port** while still keeping the flashcart for ROM execution.

The current V2 direction is:

- keep the flashcart in the cartridge slot
- use the expansion port for a better external hardware interface
- likely use an RP2040 on the external side

The currently targeted signals are:

- `CPU D0..D7`
- `A15`
- `OUT0..OUT2`
- `/IRQ`
- `+5V`
- `GND`

One very important architectural constraint:

- the bottom expansion port does **not** expose CPU `R/W`

So V2 is not currently envisioned as a generic memory-mapped peripheral, but rather as a custom bus/handshake design using the signals that are actually available.

Supporting notes for this are in:

- [V2_EXPANSION_PORT_PIN_PLAN.md](C:/Users/mto1/Documents/NES_DEV/project-v18-DN4/V2_EXPANSION_PORT_PIN_PLAN.md)
- [V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04.md](C:/Users/mto1/Documents/NES_DEV/project-v18-DN4/V2_EXPANSION_PORT_QUICK_REFERENCE_2026-05-04.md)
- [V2_EXPANSION_PORT_BOM_2026-05-04.md](C:/Users/mto1/Documents/NES_DEV/project-v18-DN4/V2_EXPANSION_PORT_BOM_2026-05-04.md)

---

## 10. Control surface planning for V2

A long-term concern was whether a more powerful bus would force the project to abandon direct hands-on controls.

Current planning suggests that a V2 based on RP2040 can still support:

- `8` pots
- `8` rotary selectors
- `1` real switch

as long as the controls are multiplexed instead of wired one-to-one.

This was analyzed in:

- [V2_GPIO_BUDGET_AND_CONTROLS.md](C:/Users/mto1/Documents/NES_DEV/project-v18-DN4/V2_GPIO_BUDGET_AND_CONTROLS.md)

---

## 11. Current position

The project is now in a useful intermediate state:

- V1.x work has not been wasted
- the controller-port experiments produced real musical and architectural lessons
- the likely long-term solution is becoming clearer
- a transition path exists instead of a hard restart

In practical terms:

- V1.8 / V18.1 can still be used to extract value from the current hardware and software base
- V2 is increasingly likely to become the real long-term architecture

---

## 12. Main lessons learned so far

1. **Separate urgent note transport from slower musical control.**
2. **Do not stream modulation state if configuration plus local runtime will do.**
3. **The controller port is usable, but only if treated with extreme discipline.**
4. **Simple hardware timing assistance may be more valuable than more protocol cleverness.**
5. **Cart-side or bus-near ownership of the musical runtime appears to be the direction used by the most capable existing designs.**

---

## 13. What kind of feedback would help most

At this stage, the project would benefit most from feedback on:

- controller-port shift-register approaches
- expansion-port hardware usage patterns
- practical limitations of the exposed NES-001 bottom expansion-port signals
- custom bus handshakes without `R/W`
- related projects that may have solved similar problems

---

## 14. Repository structure suggestion

When publishing this on GitHub, a useful structure could be:

- `/project-v18-DN4` for current ROM/runtime work
- `/arduino/NanoNesV17DN4` and `/arduino/NanoNesV18DN4` for controller-port generations
- `/docs` or `/research` for:
  - project history
  - architecture notes
  - external project research
  - V2 BOM
  - expansion-port pin plan

---

## 15. Summary

APU-8 started as a controller-port MIDI instrument experiment and gradually exposed the hard difference between:

- something that merely produces sound
- and something that behaves like a robust instrument

The project now has a much clearer understanding of both:

- what the controller port can do
- and where a better bus-side architecture is likely worth the effort

That makes the next steps much less speculative than they were at the beginning.
