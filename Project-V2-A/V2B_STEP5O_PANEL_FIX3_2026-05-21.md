# V2B Step5O Panel Fix3 - 2026-05-21

## Purpose

Fix the remaining physical panel issues reported after Step5N:

- LFO target selector works once, then appears stuck.
- Attack remains too aggressive.
- Most analog controls feel slower than attack and arp speed.
- Volume still does not respond.

## Firmware Changes

- Pico firmware tag: `t44-v2b-step5o-panelfix3`.
- LFO target selector is now fixed active-low with internal pull-ups.
- Removed the previous pull-up / pull-down polarity flip during scans.
- LFO target falls back to pitch only if no single selector line is readable.
- Attack panel curve is further softened and capped to `0..3`.
- Analog panel filter changed from 75/25 smoothing to 50/50 smoothing for faster control response.

## MCP3008 Decoupling

Recommended physical decoupling:

- `100 nF` ceramic from MCP3008 `VDD pin 16` to `DGND pin 9`, directly at the IC.
- `100 nF` ceramic from MCP3008 `VREF pin 15` to `AGND pin 14`, directly at the IC.
- Optional but recommended `10 uF` electrolytic or tantalum from the local `3.3V` rail to `GND` near the MCP3008.

## Volume Note

Volume cannot be fixed in firmware if MCP3008 `CH0` does not move in `PV`.

Check:

- slider end 1 to `3.3V`
- slider end 2 to `GND`
- slider wiper to MCP3008 `CH0`
- MCP3008 `VDD` and `VREF` to `3.3V`
- MCP3008 `AGND` and `DGND` to Pico/module ground
