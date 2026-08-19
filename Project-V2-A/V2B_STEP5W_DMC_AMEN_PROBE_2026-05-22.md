# V2B Step5W DMC Amen Probe

Date: 2026-05-22

## Goal

Re-enable `DMC` without undoing the stable four-voice Step5V transport.

This is intentionally a conservative DMC return:

- `CH11` is trigger-only.
- `CC36 DMC pitch` remains ignored.
- `P1/P2/TRI/NOISE` transport is unchanged.
- `DMC` uses one full-byte `AUX` trigger instead of a two-byte meta/value pair.

## Sample Bank

Source folder:

`C:\Users\mto1\Documents\NES SAMPLES\AMENBREAK-SLIces`

Generated files:

- `v2b_step5_p1p2trinoisedmc_ctrl_midi\dmc_samples.s`
- `v2b_step5_p1p2trinoisedmc_ctrl_midi\dmc_samples.h`

Mapping:

- `CH11 note 36` = sample id `00`
- `CH11 note 37` = sample id `01`
- ...
- `CH11 note 61` = sample id `25`

The 26 Amen `.dmc` files fit in the `$F000..$FFFA` DMC bank.

## Transport Rule

`TRI` remains authoritative through `STATUS_AUX_IS_TRI`.

If `STATUS_AUX_IS_TRI=0` and `AUX_FULL=1`, the ROM decodes:

- `0x40..0x5F` as `DMC` trigger sample `0..31`
- `0x60..0x7F` as reserved `DMC` control bytes
- other full-byte controls as `NOISE` voice controls

If `AUX_FULL=0`, the event is a compact `NOISE` note/gate packet.

This keeps `TRI`, `NOISE`, and `DMC` separated without borrowing `P1/P2`.

## Important Guard

The ROM now preserves the DMC enable bit when triangle gate changes occur.

This matters because old triangle writes to `$4015` could accidentally stop an active sample. `trigger_dmc()` still clears then re-enables the DMC bit when starting a new sample, which is required by the NES APU.

## Expected Test

- Flash `FW=t52-v2b-step5w-dmcamen`.
- Use ROM `V2_MIDI_IN_P1P2.nes` from the same step folder.
- Test `CH11` notes `36..61` solo first.
- Then test `P1/P2/TRI/NOISE` without DMC to confirm no regression.
- Then add sparse DMC hits under load.

Success criteria:

- no `TRI -> DMC` substitution
- no `TRI` sustain regression
- no sample trigger from `NOISE` timbre/control packets
- DMC may add AUX load, but should not corrupt voice identity
