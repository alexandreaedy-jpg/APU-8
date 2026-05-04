# V18.1 Action Plan

## Goal

Build a `V18.1` transport that is closer to a real NES controller / shift-register device so that:

- note triggers are precise
- note parsing is robust
- `D4` vibrato presets are no longer polluted by note traffic
- `D3` ADSR data stays out of the urgent trigger path

`V17` remains only as a fallback snapshot, not as the product target.

## Current diagnosis

The main problem in `V18` is not just tuning. The current design still behaves too much like a custom packet bus and not enough like a true controller-port shift device.

Symptoms that point to this:

- no notes at all in some builds
- random cross-triggering between voices
- envelopes becoming incoherent
- TRI / NOISE appearing to hit pulse channels

This strongly suggests frame/bit desynchronization on the live transport path.

## Design principles for V18.1

1. `D0` becomes the only urgent event lane.
2. `D3` and `D4` must never be required for note-on / note-off precision.
3. The NES must be able to read a stable, controller-like serial stream without custom "peek" logic.
4. The Nano must prepare state ahead of time and expose it like a shift register, not compute protocol behavior at the critical moment.
5. `D4` stays slow-latched config only.

## Scope split

### Phase 0: Freeze and isolate

- Keep current `V18` files as research history.
- Do not reuse the current mixed `pending + packet` experiment as the final model.
- Start `V18.1` as a clean sub-branch of the `V18` idea, not of the current broken runtime behavior.

### Phase 1: Rebuild only the D0 trigger lane

Target:

- restore stable note reading first
- ignore `D3/D4` in the critical path

Actions:

- define a minimal `D0` event frame
- keep only `P1`, `P2`, `TRI` at first
- keep `NOISE` out until the transport is proven stable
- do not include ADSR or LFO data in this frame

Candidate frame:

- byte 0: header A
- byte 1: header B
- byte 2: P1 trigger
- byte 3: P1 note/gate
- byte 4: P2 trigger
- byte 5: P2 note/gate
- byte 6: TRI trigger
- byte 7: TRI note/gate

This preserves the compact 8-byte format already explored, but the implementation must behave like a controller-compatible shift stream.

### Phase 2: Make the Nano behave like a shift register

Target:

- the Nano should expose prebuilt bits, not interpret a custom live protocol during reads

Actions:

- pre-latch the whole D0 frame outside the critical ISR path
- on `LATCH`, only reset bit position
- on `CLK`, only advance to the next bit
- no packet rebuild inside the edge-critical path
- no extra "presence protocol" unless it behaves exactly like a controller-safe first bit

Important rule:

- if the design still needs a custom non-controller "pending" flag to work, the design is still too clever and not controller-like enough

### Phase 3: Simplify the ROM fast path

Target:

- ROM should do one thing well: read a stable D0 frame

Actions:

- remove special-case presence heuristics from the fast path
- reduce the fast path to:
  - latch
  - read 8-byte D0 frame
  - validate header
  - apply only the targeted voice changes
- keep `D3/D4` fully outside this path for now

### Phase 4: Reintroduce D3 slowly

Target:

- restore `ADSR + duty` without touching trigger precision

Actions:

- slow-poll `D3`
- never require `D3` to decode note events
- verify that moving ADSR pots does not affect note parsing

### Phase 5: Reintroduce D4 slowly

Target:

- restore preset vibrato config only after D0 is stable

Actions:

- keep `D4` as preset/config lane only
- no continuous rate/depth streaming
- only accept stable repeated packets
- verify that note traffic no longer perturbs vibrato state

## Testing order

1. `D0` only
2. `D0 + TRI`
3. `D0 + D3`
4. `D0 + D4`
5. `D0 + D3 + D4`
6. add `NOISE` last

Do not skip steps. If a step fails, stop there and fix that layer before adding the next.

## Success criteria

### D0 success

- repeated notes are read reliably
- `P1`, `P2`, `TRI` do not cross-trigger
- no random envelope corruption caused by note traffic

### D3 success

- ADSR moves are stable
- ADSR changes do not alter note parsing

### D4 success

- vibrato presets remain stable while other notes are playing
- no audible perturbation from unrelated note traffic beyond the unavoidable musical limits

## Anti-goals

Do not try to solve these during Phase 1:

- perfect LFO for TRI
- final NOISE transport
- continuous analog vibrato depth/rate
- advanced optimization of ROM timing

Those are downstream goals. First priority is a sane transport.

## Recommended immediate next step

Implement a `V18.1 D0-only` pass:

- Nano:
  - real `CLK`
  - prelatched 8-byte D0 frame
  - no special D3/D4 handling in the ISR
- ROM:
  - read only D0 in fast path
  - validate header
  - apply `P1/P2/TRI`
  - leave `D3/D4` temporarily disabled

If that step does not become stable, then the remaining answer is likely hardware-assisted shifting, not more software protocol complexity.
