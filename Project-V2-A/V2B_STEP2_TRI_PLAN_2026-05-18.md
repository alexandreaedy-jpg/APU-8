# V2B Step2 TRI Plan

Date: 2026-05-18

## Goal

Add `TRI` to the validated V2B edge-driven transport without regressing:

- P1 timing
- P2 timing
- startup behavior
- groove stability at high BPM

## Constraint

The validated `P1/P2` V2B pulse-only protocol already uses the full 3-bit opcode space:

- `STATUS`
- `P1_LO`
- `P1_HI`
- `P2_LO`
- `P2_HI`
- `ACK_P1`
- `ACK_P2`
- `IDLE`

So `TRI` cannot be added by "just one more opcode".

## Recommended Direction

Move from fixed `P1/P2` slots to compact dynamic voice slots while keeping the edge-first philosophy.

Recommended next protocol shape:

- `STATUS`
- `SLOT0_TAG`
- `SLOT0_LO`
- `SLOT0_HI`
- `SLOT1_TAG`
- `SLOT1_LO`
- `SLOT1_HI`
- `ACK_BATCH`

Where:

- each slot describes one pending voice edge
- the tag identifies `P1`, `P2`, or `TRI`
- one service pass can deliver up to 2 edges

## Why This Direction

It keeps the key V2B win:

- cheap note edges
- no global snapshot
- no heavy generic register walk

But it scales to:

- `P1`
- `P2`
- `TRI`

without going back to the old transport cost that broke groove.

## Step2 Scope

The next implementation should do only:

- `P1`
- `P2`
- `TRI`
- note/gate edge transport only

Do not add yet:

- `NOISE`
- `DMC`
- replaceable control-state packets

Etat au 2026-05-18 :

- premiere implementation codee dans :
  - `Project-V2-A\v2b_step2_p1p2tri_edge_midi`
  - `arduino\PicoNesV2B_Step2P1P2TriEdgeMidi`
- cible courte :
  - `midi-v2b2`

## Success Criteria

- `P1` solo still clean
- `P2` solo still clean
- `TRI` solo clean
- `P1 + P2 + TRI` stays musical at high BPM
- no startup mute regression
- no fake low-note mismatch if firmware/ROM are mixed
