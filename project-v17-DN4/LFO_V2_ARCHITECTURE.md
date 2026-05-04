# LFO V2 Architecture

## Intent

This document resets the `D4` LFO design from zero.

The goal is not to tune the current LFO again. The goal is to replace the
current "live D4 modulation" concept with a cleaner design:

- `D0` stays responsible for note/gate/trigger events
- `D3` stays responsible for ADSR + duty
- `D4` becomes a **slow configuration channel only**
- the NES ROM owns the actual oscillator phase and timing
- the LFO must no longer be clocked or dirtied by note traffic on `D0`

This is the base design we should follow for the next implementation pass.

## Why V1 Fails

The current LFO concept on `D4` has three structural problems:

1. `D4` is read inside the same fast runtime loop as notes, ADSR and APU writes.
   Even with better timing, that keeps the modulation tied to unrelated work.

2. The same "depth" concept is reused for very different targets:
   - pitch
   - amp
   - duty
   - triangle pitch

   Those targets do not want the same mapping at all.

3. The modulation is treated as if `D4` were a live modulation stream, while
   in practice the best part of the system is when `D4` behaves like stable
   configuration and the ROM does the rest alone.

So the right redesign is:

- `D4` transports **configuration**
- the ROM runs the oscillator
- `D0` note-on optionally resets phase for the selected voice

## High-Level Rules

### Rule 1

`D4` must never carry "instantaneous modulation state".

It carries only:

- target
- depth
- rate
- sync mode

### Rule 2

The ROM must latch a `D4` packet only when it is stable.

Recommended acceptance rule:

- valid header
- valid checksum
- same packet received twice in a row

Only then update the live LFO config.

### Rule 3

The oscillator phase must advance from a ROM-owned clock only.

Recommended first clock source:

- `FRAME_CNT1` frame delta

The phase must not depend on:

- `packet_seen`
- note traffic
- ADSR ticks
- release transitions

### Rule 4

Targets must be handled by separate render logic:

- `AMP`: proper tremolo
- `PITCH`: dedicated micro-vibrato
- `DUTY`: discrete wobble, not analog-like depth
- `TRI`: pitch-only, shallow, optional

## NESizer2 Lessons Worth Keeping

After reviewing the reference project
[NESizer2](https://github.com/Jaffe-/NESizer2), the most relevant ideas for
our branch are not its exact transport or bus timing, but its internal audio
organization.

### 1. Separate timing domains

The strongest idea in NESizer2 is the task split:

- LFO update
- envelope update
- modulation calculate
- modulation apply
- APU register refresh

Those parts do not all run inside one monolithic loop. This is the main reason
its modulation stays cleaner under load.

For our project, we cannot copy the whole scheduler literally, but we should
preserve the same principle:

- `D0` = event transport
- `D3` = slow control transport
- `D4` = slow LFO config transport
- ROM LFO runtime = independent from note traffic

### 2. Modulation is not just "subtract depth"

NESizer2 does not implement tremolo as a crude on/off attenuation. Its
modulation layer scales the audio parameter from an internal LFO value.

That means:

- the LFO oscillator owns the waveform
- the modulation layer owns the target-specific mapping

This is the correct model for us too. In practice:

- `AMP` can use multiplicative tremolo
- `PITCH` needs its own micro-vibrato mapping
- `DUTY` needs a discrete mapping, not analog depth

### 3. Pitch must remain a special case

NESizer2 routes pitch modulation through proper period logic instead of
treating it like a direct volume wobble.

This matches the NESdev guidance:

- pulse high-register writes reset phase/envelope
- triangle pitch writes are their own special case

So `PITCH` must remain a dedicated phase-2 mode in our branch.

### 4. Phase 1 scope must stay narrow

The cleanest path for `project-v17-DN4` is:

- `P1/P2`
- `AMP`
- stable rate/depth
- no fake support for `PITCH`, `DUTY`, or `TRI`

Unsupported targets should be encoded as OFF, not "best effort".

## D4 V2 Packet

We keep `PACKET_SIZE = 12` for compatibility with the current transport, but
the meaning becomes much simpler.

### Header

- `byte 0 = 0xD4`
- `byte 1 = 0xB4`
- `byte 2 = seq`

`seq` increments when any LFO config changes.

### Per-Voice Config

Each voice uses two bytes.

#### Voice config byte A

- bits `7..6`: mode
- bits `5..4`: target
- bits `3..0`: depth

#### Voice config byte B

- bits `7..4`: rate
- bits `3..0`: reserved

### Layout

- `byte 3 = P1 cfg A`
- `byte 4 = P1 cfg B`
- `byte 5 = P2 cfg A`
- `byte 6 = P2 cfg B`
- `byte 7 = TRI cfg A`
- `byte 8 = TRI cfg B`
- `byte 9 = global flags`
- `byte 10 = reserved`
- `byte 11 = checksum`

### Target values

- `0 = off`
- `1 = amp`
- `2 = pitch`
- `3 = duty`

For `TRI`, only `off` and `pitch` are meaningful. `amp` and `duty` must be
ignored by the ROM.

### Mode values

- `0 = free-run`
- `1 = retrig on note-on`
- `2 = one-shot` reserved
- `3 = reserved`

### Global flags

`byte 9`:

- bits `1..0`: clock source
- bits `3..2`: reserved
- bit `4`: require double-packet latch
- bit `5`: reserved
- bit `6`: `CLK` available
- bit `7`: reserved

Clock source:

- `0 = frame-sync`
- `1 = faster frame divider`
- `2 = reserved`
- `3 = reserved`

### Checksum

`byte 11` is the XOR of bytes `0..10`.

This gives the ROM a cheap way to reject torn reads without inventing a more
complex transaction protocol.

## ROM-Side Model

The ROM should stop thinking in terms of "current D4 bytes" and instead own
these two structures per voice:

```c
typedef struct {
    unsigned char target;
    unsigned char mode;
    unsigned char depth;
    unsigned char rate;
} LfoConfig;

typedef struct {
    unsigned char phase;
    unsigned char last_frame;
    signed char offset;
} LfoState;
```

Recommended arrays:

- `p1_lfo_cfg`, `p1_lfo_state`
- `p2_lfo_cfg`, `p2_lfo_state`
- `tri_lfo_cfg`, `tri_lfo_state`

### Update path

1. Read raw `D4` packet.
2. Validate header + checksum.
3. Compare to previous candidate packet.
4. If identical twice, decode into `LfoConfig`.
5. The fast loop reads only `LfoConfig`, never the raw packet.

### Phase advance

Per voice:

- compute `frame_delta = FRAME_CNT1 - last_frame`
- `phase += phase_step(rate) * frame_delta`
- compute render offset from `phase`

This is the part that must stay independent from notes and ADSR.

## D0 Interaction

`D0` remains the note/gate channel.

On note-on:

- if the voice mode is `free-run`, keep phase
- if the voice mode is `retrig on note-on`, reset phase

On note-off:

- do not stop or restart the LFO
- keep modulation running through the release

This is how we prevent the old "gap at release" behavior from returning.

## Target-Specific Rendering

### AMP

This is the primary target for LFO v2.

Recommended behavior:

- use a dedicated tremolo depth table
- do not reuse pitch depth directly
- keep the base envelope intact, modulate the rendered level only

### PITCH

Pitch must be treated as a special micro-vibrato engine.

Recommended behavior:

- low-byte timer offsets only when possible
- shallow table with musically chosen deltas
- no aggressive writes that keep re-kicking the high register behavior

This should be the second target we implement after `AMP`.

### DUTY

Duty should no longer pretend to be continuous.

Recommended behavior:

- discrete wobble around the base duty
- thresholded motion
- if it still sounds bad after that, keep it as an experimental target only

### TRI

Triangle should only support shallow pitch vibrato.

Recommended behavior:

- reuse the pitch target
- with a smaller depth table than pulses

No triangle amp LFO. No triangle duty LFO.

## CLK Wire

We should plan for `CLK` from the start.

### Recommendation

- yes: wire `CLK` physically now
- no: do not make the first LFO v2 implementation depend on it

### Why it is still useful

If the current `OUT`-only handshake keeps showing limits later, `CLK` gives us
a deterministic shift reference for the Nano.

That helps the **transport**, but it does not replace the new LFO concept.

So the right order is:

1. fix the architecture first
2. use `CLK` only if transport robustness still needs help

### How to expose it in v2

In Nano code:

- declare `CLK` as an optional input
- do not use it yet for the first LFO v2 pass
- set `global flags bit 6` only when the code path actually uses it

## Implementation Order

### Phase 1

Implement `LFO v2` for `AMP` only on `P1` and `P2`.

Success criteria:

- no D0/D3 pollution
- stable rate
- useful depth pot
- proper per-voice separation

### Phase 2

Add `PITCH` micro-vibrato on `P1` and `P2`.

Success criteria:

- clearly audible max depth
- subtle low depth
- no delayed start or resume around release

### Phase 3

Add `TRI` pitch vibrato only.

If it is still musically poor, keep it optional or disable it.

### Phase 4

Try `DUTY` only after `AMP` and `PITCH` are good.

If `DUTY` still sounds bad, keep the target in the protocol but treat it as
experimental.

## Migration Notes

Current rollback state worth keeping:

- ROM hash:
  - `BB9C395111550817C48E8B15BE40F7FFE6D4B34E8966A1ECBB051FF670272B3C`

When we start coding LFO v2:

1. do not touch `D0`
2. do not touch `D3`
3. replace only:
   - Nano `writeD4PacketFromState()`
   - ROM `D4` decode path
   - ROM LFO runtime
4. remove the leftover LFO-related nibbles still piggybacked on the current
   `D0` voice packet so `D0` becomes strictly note/gate/trigger again

This is the guardrail that should stop the redesign from spilling into ADSR
again.
