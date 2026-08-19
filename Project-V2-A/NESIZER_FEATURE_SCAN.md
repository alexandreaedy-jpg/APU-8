# NESizer2 feature scan for V2_A

## Features worth porting

### Pitch/modulation core
- Three global LFOs with selectable waveforms: sine, ramp up, ramp down, triangle, square.
- Per-channel LFO routing matrix: SQ1/SQ2/TRI/NOISE can each receive LFO1, LFO2, LFO3 at independent depths.
- Pitch bend per channel with configurable semitone range.
- Detune and coarse octave shift per pitched channel.
- Pitch envelope modulation: envelope can push pitch, useful for NES percussion and attack transient effects.
- Volume modulation by LFO3 for SQ1/SQ2/NOISE tremolo.

### Portamento / glide
- SQ1/SQ2/TRI glide uses a target note and a current pitch position in 1/64-semitone units.
- The glide step updates toward the target instead of jumping timer values directly.
- This fits V2_A well because Step22/23 already proved that musical pitch offsets are safer than raw timer sweeps.

### Note assignment
- Mono/poly voice allocation across channels.
- Split keyboard support with lower/upper groups.
- Note stack for monophonic legato: when a held note is released, the previous held note resumes.

### Sequencer and sync
- 16-step patterns for five lanes: SQ1, SQ2, TRI, NOISE, DMC.
- Internal tempo or external MIDI clock.
- MIDI Start, Stop, Continue, and Clock drive the sequencer when external clock is enabled.
- Patterns include note and length per step.

### DMC samples
- DMC sample slots addressed from MIDI notes.
- Sample loop flag.
- SysEx sample loading exists in NESizer, but this is probably later for V2_A because ROM sample memory and transfer path are separate problems.

## Lessons for V2_A

- Do not add more raw timer modulation. NESizer converts musical pitch units to timer values after summing modulation sources.
- Step23 should evolve into a small pitch engine:
  `base note + portamento + LFO + bend + detune/coarse -> timer`.
- Keep LFO clocking independent from bus transaction timing. Step23 proved that bus-driven LFO phase creates either invisible or extreme modulation.
- Start with P1/P2, then extend to TRI once stable. Noise pitch modulation is less critical and more special-case.
- ARP is not really a NESizer core feature; ChipMaestro is the better reference for arpeggio direction/speed behavior.
- Sync is a NESizer strength: MIDI clock should drive future arp/sequencer ticks rather than audio timer updates.

## Suggested next steps

1. Step24: central pitch engine for P1/P2.
   Keep audible behavior close to Step23, but refactor pitch so LFO is one source among others.

2. Step25: portamento/glide test.
   Add one packed control for glide time and create a slow note pattern to hear slide vs instant note jumps.

3. Step26: pitch bend test.
   Add bend amount/range and validate smooth bend on P1/P2.

4. Step27: arpeggiator prototype.
   Use ChipMaestro-style arp direction: up, down, ping-pong. Clock it internally first.

5. Step28: MIDI clock sync.
   Use MIDI Clock/Start/Stop/Continue to drive arp/sequencer ticks, not audio modulation ticks.

6. Later: LFO waveforms and modulation matrix.
   Expand from one shared vibrato into LFO waveform + per-channel routing.
