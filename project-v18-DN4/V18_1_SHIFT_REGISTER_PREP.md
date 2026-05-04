# V18.1 Shift Register Prep

## Objective

Push the NES controller port as far as it can go by making `D0` behave like a real controller-style serial source, instead of a software-timed packet bus.

This document prepares the hardware direction before buying parts.

## Main conclusion

For `V18.1`, the urgent note lane should move toward a **true shift-register output** on `D0`.

That means:

- the NES owns `LATCH` and `CLK`
- the external hardware exposes a stable byte
- the Nano prepares event data ahead of time
- the NES clocks it out like a controller

## Which shift register?

### Best NES-native choice: `4021`

Why:

- it is the actual controller-style part used by standard NES pads
- its control behavior matches the NES controller model naturally
- it is the cleanest conceptual fit for `OUT0/LATCH + CLK + D0`

NESdev references:

- [4021](https://www.nesdev.org/wiki/4021)
- [Standard controller](https://www.nesdev.org/wiki/Standard_controller)

Important behavior:

- latch/load happens when the control line is asserted
- bits are shifted on the clock transition
- after 8 bits, the standard controller behavior is well understood

### Practical substitute: `74HC165`

Why:

- easier to source in practice
- fast enough by a huge margin for NES controller timing
- explicitly acknowledged by NESdev as a suitable substitute

Reference:

- [4021 NESdev page](https://www.nesdev.org/wiki/4021)
- [74HC165 Nexperia datasheet](https://assets.nexperia.com/documents/data-sheet/74HC_HCT165.pdf)

Important caveat:

- `74HC165` uses an **active-low parallel load** input (`PL`)
- the NES controller-style latch logic is conceptually closer to the `4021`
- so with `74HC165`, we likely need an **inversion stage** for the latch/load behavior

## Critical discovery: one PISO chip alone is not enough

The current Nano pin map is almost saturated:

- `D12` = NES `LATCH`
- `D13` = NES `CLK`
- `D2/D3/D4` = current `D0/D3/D4` outputs
- `D5..D11` = selectors and LFO target switches
- `A0..A5` = active controls
- `A6/A7` = reserved analog controls
- `D0/RX` = MIDI input

So even if we add a `4021` or `74HC165`, we still need a way to **load 8 event bits into it**.

That means a single PISO chip is only enough if:

- we temporarily sacrifice several Nano pins for a proof of concept, or
- we add a second stage that lets the Nano load the byte with fewer pins

## Two realistic hardware paths

### Path A: quick proof-of-concept

Use:

- `1x 4021` or `1x 74HC165`

Method:

- temporarily dedicate 8 Nano outputs to the event byte
- use the PISO only as the final controller-facing shifter

Pros:

- easiest to understand
- easiest to debug

Cons:

- the current Nano pin budget probably cannot support this without disconnecting controls
- good for bench proof, not likely the final wiring

### Path B: scalable path

Use:

- `1x 74HC595` or equivalent serial-to-parallel register
- `1x 4021` or `1x 74HC165` as the controller-facing parallel-to-serial stage

Method:

- Nano serial-loads the event byte into the `74HC595`
- `74HC595` outputs feed the 8 parallel inputs of the `4021/74HC165`
- NES `LATCH` loads the PISO stage
- NES `CLK` shifts the bits out on `D0`

Pros:

- much closer to a real product architecture
- reduces Nano pin pressure dramatically
- lets the PISO stage behave like a true controller-facing register

Cons:

- more components
- more wiring

## Recommended shopping list for Saturday

### Recommended primary parts

- `2x CD4021BE` or equivalent `4021`
- `2x 74HC165N` or `74HCT165N`
- `2x 74HC595N`

Why buy both families:

- `4021` is the clean NES-native option
- `74HC165` is the pragmatic fallback/substitute
- `74HC595` may become necessary because the Nano does not have 8 free output pins for direct loading

### Support parts

- `1x 74HC14N` or `74HC04N`
  - useful if we prototype with `74HC165` and need latch inversion/cleanup
- several `100 nF` decoupling capacitors
- a few `1 kΩ` and `10 kΩ` resistors
- breadboard jumpers

## First recommended prototype

Prototype goal:

- make `D0` stable first
- do not try to bring `D3/D4` into the shift-register experiment yet

Recommended first prototype:

- `D0` only through `4021`
- `D3/D4` stay Nano-direct and slow
- `P1/P2/TRI` event frame remains 8 bytes
- `NOISE` stays out for now

Reason:

- this isolates the real problem: urgent note timing
- if this does not become stable, then the controller-port limit is elsewhere

## Suggested signal roles

### With `4021`

- NES `OUT0/LATCH` -> `4021` parallel/serial control
- NES `CLK` -> `4021` clock
- `4021` serial out -> NES port 2 `D0`
- serial in tied so post-8-read behavior is known and reproducible
- Nano provides the 8 parallel inputs indirectly or directly

### With `74HC165`

- NES `OUT0/LATCH` -> inverted to `PL`
- NES `CLK` -> `CP`
- `Q7` -> NES port 2 `D0`
- serial input tied for known post-8-read behavior
- Nano provides the 8 parallel inputs indirectly or directly

## Software implication

The software target should become:

- fast path reads only `D0`
- `D0` is interpreted as a controller-like event frame
- `D3` and `D4` return only after `D0` is proven stable again

So the hardware and software plans now align:

- `V18.1 D0-only`
- then `D3`
- then `D4`

## Recommendation

If only one family is available and we want the cleanest first try:

- buy `4021`

If we want the most flexible basket and do not mind testing alternatives:

- buy `4021 + 74HC165 + 74HC595 + 74HC14`

That gives us enough room to test both:

- a NES-native controller-style shifter
- and a more practical modern logic chain
