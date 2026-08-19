## Suggested thread title

APU-8: MIDI-controlled NES APU experiment, controller-port limits, and a possible expansion-port V2

## Draft post

Hello everyone,

I have been working on a NES/Famicom MIDI instrument project that I currently call **APU-8**, and I would really appreciate feedback from people who have experimented with controller-port I/O, custom expansion-port hardware, or cart-side synth runtimes.

The detailed project notes and code will be posted on GitHub here:

`<PUT YOUR GITHUB PROJECT URL HERE>`

### Goal

The goal is to turn a real NES into a playable external MIDI-controlled sound module with:

- Pulse 1
- Pulse 2
- Triangle
- Noise
- eventually DPCM / extra behavior where practical

The main design goal is not just "MIDI in, sound out", but something that feels like a real instrument:

- low note latency
- stable note triggers
- usable per-voice controls
- modulation that does not fall apart under note traffic
- ideally a hardware workflow that is not totally dependent on a PC UI

### What I built first

The first serious hardware/software path used:

- a flashcart / EverDrive to run a custom ROM
- an Arduino Nano receiving MIDI DIN and local controls
- the NES controller port as a custom transport

I ended up with a multi-lane controller-port design:

- `D0` = note / gate / trigger lane
- `D3` = ADSR + duty / slower controls
- `D4` = LFO / modulation config lane

That architecture gradually evolved through several versions.

### What worked

One important milestone was a **stable V16** state where:

- `D0` notes were stable
- `D3` ADSR + duty were stable
- `D4` LFO was stable
- ROM and Nano behavior were aligned

In other words, the separate-lane idea was not nonsense. It could be made to work to a point.

Another important lesson was that `D4` became much more reliable when it stopped behaving like a live modulation stream and instead became a **slow config lane**.

This led to a useful V17 result:

- vibrato presets worked well when sent as **stable preset/config changes**
- this was far better than continuously streaming analog rate/depth values

That part actually felt promising musically.

### What did not work well

The remaining problems were more structural:

1. **Note traffic still perturbed modulation**

Even when `D4` itself was no longer noisy, note traffic from other lanes still affected the audible result, because too much runtime work was still sharing the same live path.

2. **The controller port is very unforgiving**

As soon as I pushed harder on timing precision, I ran into:

- note loss
- cross-triggering between channels
- envelope corruption
- builds with no sound at all
- behavior that strongly suggested bit/frame desynchronization

3. **Trying to make the controller port behave like a rich packet bus was fragile**

My current conclusion is that the controller port really wants a much more disciplined model:

- one tiny urgent lane
- slower config lanes outside the critical path
- local runtime ownership inside the ROM

### What I learned from existing projects

I reviewed the public material I could find for:

- MIDINES
- ChipMaestro
- NESizer2
- Famimimidi
- SynthNes

The broad lessons I took from them are:

- **ChipMaestro** shows the value of short, targeted, event-like communication and not overloading the transport.
- **NESizer2** strongly reinforces separation of timing domains: input, modulation, envelopes, and rendering should not all fight in one ad-hoc loop.
- **Famimimidi** seems to get much of its power from owning a dedicated cart-side synth runtime instead of trying to push too much through a constrained input path.

This pushed me to stop treating the controller port like a general-purpose bus.

### Current V18 / V18.1 direction

The current redesign idea is:

- keep `D0` as the only urgent lane
- keep `D3` and `D4` out of the note-critical path
- make the transport behave more like a real controller / shift-register device

I even built a prototype around:

- `74HC595`
- `4021`

so that the NES would read a prebuilt serial stream instead of relying on the microcontroller to improvise bit timing in real time.

The idea makes sense, but I have not yet reached a final stable musical result from that branch.

### Why I am now considering an expansion-port V2

The more I worked on this, the more it looked like the controller port might simply be the wrong place to carry a richer synth-control architecture.

So I have started planning a **V2** based on the **NES-001 bottom expansion port** while still keeping a flashcart in the cartridge slot.

The current V2 concept is:

- keep the flashcart for the ROM/runtime
- use the expansion port for a better hardware interface
- likely use an RP2040 on the external side

The exposed signals I am currently interested in are:

- `CPU D0..D7`
- `A15`
- `OUT0..OUT2`
- `/IRQ`
- `+5V`
- `GND`

One important limitation I noticed is that the bottom expansion port does **not** expose CPU `R/W`, so this would not be a generic memory-mapped peripheral in the usual sense. It would need a more custom handshake/latch design.

### What I would love feedback on

1. **Has anyone here built a custom device on the NES-001 bottom expansion port while still running code from a flashcart or cartridge?**

2. **Given that `R/W` is not exposed, what would you consider the cleanest protocol strategy using only the expansion-port signals that are available?**

3. **For controller-port experiments, has anyone successfully used a 4021/165-style external serializer for non-controller data under real musical load?**

4. **Are there any practical expansion-port gotchas I should be especially careful about beyond the well-known `/IRQ` series resistor advice?**

5. **If anyone knows of public documentation, teardown notes, or architecture clues for Famimimidi or similar projects, I would be very interested.**

### Why I am posting now

I feel like I have reached the point where:

- the controller-port experiments have produced real lessons
- the project is no longer just a vague idea
- but I am also close to an architectural pivot

So this seemed like the right moment to ask for informed feedback before I sink too much time into the wrong next step.

If there is interest, I can post:

- the current ROM/Nano code
- wiring diagrams
- notes on V16 / V17 / V18 behavior
- the expansion-port V2 pin plan and BOM

Thanks in advance for any advice.
