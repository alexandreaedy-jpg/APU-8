# V2B Step5N Panel Fix2 - 2026-05-20

## Purpose

Recover the physical panel behavior after `STEP2B-Panel_Control_test6`:

- LFO disappeared because the LFO target selector stayed at `LM=0`.
- Attack was still far too aggressive because the panel value jumped to high nibbles quickly.
- Volume remained non-functional because MCP3008 channel 0 stayed at `0` in the log.

## Changes

- Pico firmware tag: `t43-v2b-step5n-panelfix2`.
- LFO target selector now tries active-low and active-high reads.
- If no single LFO target is readable, the panel falls back to `Pitch` instead of disabling LFO.
- Panel attack is heavily softened and capped to a safe `0..5` range.
- Panel volume still ignores `0` to prevent accidental mute while MCP channel 0 is stuck low.

## Expected Log

- `FW=t43-v2b-step5n-panelfix2`
- `PL=PIT` by default, even if the LFO selector is electrically ambiguous.
- `LM=1`, `2`, or `4` when the LFO selector is detected cleanly.
- `PV` second digit for attack should no longer exceed `5`.

## Known Hardware Check

If volume still does not move, check the MCP3008 `CH0` wiring:

- slider end 1 to `3.3V`
- slider end 2 to `GND`
- slider wiper to MCP3008 `CH0`
- common ground between MCP3008 and Pico

In test6, `PV` first digit stayed `0` for the whole run, so firmware had no usable volume value to apply.
