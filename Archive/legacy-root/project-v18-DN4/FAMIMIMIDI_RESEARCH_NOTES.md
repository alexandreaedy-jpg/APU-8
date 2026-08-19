## Famimimidi Research Notes

Date: 2026-05-04

### Goal

Collect public documentation and user reports on Famimimidi to extract lessons for
APU-8, especially around:

- transport architecture
- note precision / latency
- modulation strategy
- controller interference
- noise / DPCM behavior

### Strong public sources found

1. Official-ish implementation chart, Japanese, v0.7a:
   https://hongera.sakura.ne.jp/Famimimidi_implement_chart_v07a_0816.pdf
2. Archived Catskull product page:
   https://web.archive.org/web/20200511172113/https://catskullelectronics.com/famimimidi
3. Maker Faire Tokyo 2015 listing:
   https://makezine.jp/event/makers2015/nankahennamono_ensousuru/
4. ChipMusic user thread:
   https://chipmusic.org/forums/topic/24277/famimimidi-nes-edition-anyone-used-it-yet/
   https://chipmusic.org/forums/topic/24277/famimimidi-nes-edition-anyone-used-it-yet/page/2/
5. Community rewritten manual (2023), not canonical but useful:
   https://drive.google.com/file/d/1Y_nA4EfMoNj6acvg44H7IxJyirCcRo6X/view?usp=sharing
6. Practical DAW workflow report:
   https://joymecha.blog.fc2.com/blog-entry-31.html

### What is clearly confirmed

- Famimimidi is a cartridge-based MIDI interface for NES/Famicom, not a controller-port hack.
- The archived product page explicitly says there is no on-screen interface.
- The Japanese v0.7a chart says video output is disabled and the console outputs sound only.
- The same chart maps MIDI channels to voices:
  - ch1 pulse 1
  - ch2 pulse 2
  - ch3 triangle
  - ch4 noise
  - ch5 DPCM
- The chart also confirms direct SysEx writes to APU addresses `$4000` to `$4017`.
- It exposes many synth-like parameters directly via MIDI CC:
  - volume / expression
  - sweep
  - ADSR-like envelope controls
  - modulation rate / depth / delay / waveform
  - detune
  - pitch bend sensitivity
- The chart includes custom modulation waveforms and user-defined waveform data via SysEx.
- The official note/noise map and DPCM map are public.

### What looks very likely, but comes from community docs or user reports

- Extra live-performance features beyond the v0.7a chart:
  - portamento
  - tremolo
  - arpeggiator
  - breath-control assignments
  - aftertouch assignments
  - constant velocity
  - wavetable mode on channel 5
- User macros/presets seem to exist, but users reported confusion and unreliable storage behavior.
- The community manual author was unsure that custom wavetable loading was actually working as documented.

### Important architecture clues

1. It is probably fast because it is a dedicated cart synth runtime.

   The strongest clue is not hidden hardware documentation, but the behavior:

   - no on-screen interface
   - video disabled
   - controller used mainly for config
   - direct APU register access via SysEx

   This strongly suggests Famimimidi is not trying to be a "normal NES app with UI"
   while also being a MIDI synth. It is much closer to a dedicated synth firmware on a cart.

2. It keeps a lot of musical behavior inside the cart runtime.

   Instead of streaming raw register timing from the outside world, the public docs show
   a high-level control layer:

   - modulation as a named subsystem
   - envelope parameters
   - sweep
   - pitch bend
   - wavetable / DPCM behavior

   That means the external MIDI stream can stay musical instead of being a raw transport of
   tiny low-level events.

3. It still is not magic.

   Public user reports show:

   - some people found it extremely powerful and more controllable than MIDINES
   - some people reported stuck notes with merge setups
   - at least one user reported skipped notes / unstable playback outside a proper DAW workflow
   - macro behavior and wavetable behavior were not fully clear to users

### Lessons for APU-8

#### Lesson 1: Rich features do not prove a richer transport.

Famimimidi is feature-rich from the user's perspective, but that does not automatically mean
"more open" or "more documented" under the hood. The public material mostly documents the
MIDI surface, not the internal architecture.

#### Lesson 2: The biggest win seems to be architectural separation.

Famimimidi appears to avoid one of our biggest current problems:

- transport timing
- controller traffic
- modulation runtime
- voice rendering

are not all fighting in the same ad-hoc loop.

#### Lesson 3: The cart approach buys freedom.

Because it is cart-side, it can afford to:

- disable video
- own the runtime
- expose direct APU writing
- avoid controller-port serialization constraints

That is a major reason it can feel "bigger" than MIDINES or controller-port experiments.

#### Lesson 4: Even good architectures can still have workflow fragility.

Famimimidi seems stronger than MIDINES on feature depth, but the public user reports suggest:

- controller mapping can be heavy
- manual complexity is high
- some functions are hard to verify
- stability can still depend on the MIDI host path

So it is not a proof that "all cart designs are automatically perfect".

### What this means for V18 / V2

#### For V18 / controller-port path

Famimimidi does not disprove V18, but it reinforces that the critical path must be tiny and
deterministic. If we stay on the controller port, we need:

- one urgent lane
- slow lanes outside the critical path
- modulation generated locally, not streamed continuously

#### For a future V2 / expansion-port or cart-side path

Famimimidi is a strong argument that the "dedicated synth runtime closer to the bus" idea is
worth taking seriously. The public docs do not reveal the exact hardware, but the user-facing
behavior is very consistent with that strategy paying off.

### Open questions still not answered publicly

- No public schematic found.
- No public source code found.
- No confirmed teardown or PCB analysis found.
- No authoritative public explanation found for:
  - how its voice scheduling actually works
  - how the "8 poly" Maker Faire wording was achieved
  - how stable custom wavetable loading really is
  - how macros are stored internally

### Bottom line

Famimimidi does look more advanced than MIDINES at the control layer.

What is clearly documented is not "secret hardware brilliance", but rather:

- direct cart-side ownership of the runtime
- lots of built-in musical control features
- direct APU-facing behavior
- fewer compromises than a controller-port transport

That makes it a very relevant reference for APU-8, especially as an argument for keeping
musical logic local and the transport path as disciplined as possible.
