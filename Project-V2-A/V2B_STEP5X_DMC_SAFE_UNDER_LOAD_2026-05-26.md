# V2B Step5X DMC Safe Under Load

Date: 2026-05-26

## Problem

The Step5W one-byte DMC trigger range could still collide with a mis-typed `TRI` byte under heavy AUX load.

The dangerous case was especially plausible around note ends:

- `TRI` note-off values are raw note numbers.
- Some raw high notes can numerically look like a DMC trigger.
- If the status/type read is disturbed under DMC/DMA pressure, the ROM can decode a `TRI` value as a sample trigger.

## Fix

Step5X changes DMC from a one-byte trigger to a guarded pair:

- first byte: `0xFD` for samples `0..15`, or `0xFE` for samples `16..31`
- second byte: `0xE0..0xEF`, where the low 4 bits are the sample id inside that bank

A raw `TRI` byte can never naturally produce `0xFD/0xFE`, and a lone `0xE0..0xEF` value is harmless if the guard byte was missed. This makes accidental `TRI -> DMC` triggering much harder than Step5W.

## Load Policy

DMC is now lower priority than the musical lanes:

- `TRI` pending wins before starting a new DMC pair.
- `NOISE` pending wins before starting a new DMC pair.
- If a DMC pair has already sent `0xFD`, its value byte is allowed to finish immediately.
- If `TRI+NOISE` AUX backlog is too high, new DMC triggers are dropped.
- If DMC already has too much pending data, new DMC triggers are dropped.

The heartbeat adds:

- `DD=` number of deliberately dropped DMC triggers

This is intentional. Dropping a sample under overload is better than corrupting `TRI`, `NOISE`, or the groove.

## ROM Guard

The ROM only fires DMC if:

- it first receives `0xFD` or `0xFE` on the DMC path
- then receives a guarded value byte `0xE0..0xEF`

The ROM also caps live DMC playback rate to `$0D` even if the source `.prm` says `$0F`, reducing peak DMC DMA pressure during transport.

## Expected Test

- `CH11` solo notes `36..61`: samples should trigger.
- `TRI` solo short notes: no DMC sample at note start or note end.
- `TRI + DMC`: no sample should appear unless `CH11` actually plays.
- `P1/P2/TRI/NOISE + DMC` stress: DMC may drop hits under load, visible as `DD>0`, but no instrument substitution should happen.
