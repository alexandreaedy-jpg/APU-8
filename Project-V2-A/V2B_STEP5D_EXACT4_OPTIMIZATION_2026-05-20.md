# V2B Step5D Exact4 Optimization

Date: 2026-05-20

## Summary

Step5D keeps the no-DMC four-voice profile and improves headroom for dense `1/32` arp passages without dropping arp steps.

The design choice is exact transport: old arp events are not intentionally discarded or auto-throttled.

## Transport Rules

- `CH11` remains ignored and DMC remains disabled.
- Live voices are `P1`, `P2`, `TRI`, and `NOISE`.
- `AUX` carries only `TRI` or `NOISE`.
- Per-voice queue capacity is increased to 32 events.
- ROM service budget is increased to 64 events per loop.
- Arp steps remain ordered and exact.
- Note-off events remain ordered.

## CC Rules

- `NOISE` CC remains latest-value-wins.
- Existing `NOISE` control pairs can be promoted to the front of the `NOISE` queue when safe.
- A control value already waiting at the head is never preempted.
- Global CC fanout sends `NOISE` before `P1/P2` for shared controls.

## Validation

- Expected firmware: `FW=t33-v2b-step5d-exact4`.
- Expected log: `DMC=--/0`.
- `AX=` should show only `TRI`, `NOI`, or `--`.
- `1/32` arp may still build latency under impossible sustained load, but should have more headroom before overflow.
