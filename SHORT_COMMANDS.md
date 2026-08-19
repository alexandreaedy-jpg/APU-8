# Short Commands

Use these short phrases to reduce token use.

## Build / Deploy

- `status midi`: show paths for the current active Step5AR / Step5BB target
- `build midi`: build the current active MIDI ROM + Pico
- `rom midi`: build/push or push the current active MIDI ROM to `D:\game.nes`
- `flash midi`: flash the current active Pico MIDI sketch on COM6
- `deploy midi`: build ROM, push ROM, build Pico, flash Pico for the current active target
- `status midi-v2a`: show paths for the archived V2A baseline
- `deploy midi-v2a`: rebuild/reflash the archived V2A baseline
- `status midi-v2b1`: show paths for frozen V2B P1/P2 base
- `deploy midi-v2b1`: rebuild/reflash the frozen V2B P1/P2 base
- `status midi-v2b2`: show paths for experimental V2B P1/P2/TRI step
- `deploy midi-v2b2`: rebuild/reflash the experimental V2B P1/P2/TRI step
- `status midi-v2b3`: show paths for experimental V2B P1/P2/TRI/NOISE step
- `deploy midi-v2b3`: rebuild/reflash the experimental V2B P1/P2/TRI/NOISE step
- `status midi-v2b4`: show paths for experimental V2B P1/P2/TRI/NOISE/DMC probe step
- `deploy midi-v2b4`: rebuild/reflash the experimental V2B P1/P2/TRI/NOISE/DMC probe step
- `status midi-v2b5`: show paths for experimental V2B full-lane + control-lane step
- `deploy midi-v2b5`: rebuild/reflash the experimental V2B full-lane + control-lane step
- `status midi-v2b6`: show paths for Step5AB groove/AUX/arp experiment
- `deploy midi-v2b6`: rebuild/reflash Step5AB groove/AUX/arp experiment
- `status midi-v2b7`: show paths for Step5AC arp-pressure experiment
- `deploy midi-v2b7`: rebuild/reflash Step5AC arp-pressure experiment
- `status midi-v2b8`: show paths for Step5AD exact MIDI / panel lock experiment
- `deploy midi-v2b8`: rebuild/reflash Step5AD exact MIDI / panel lock experiment
- `status midi-v2b9`: show paths for Step5AE hard realtime notes experiment
- `deploy midi-v2b9`: rebuild/reflash Step5AE hard realtime notes experiment
- `status midi-v2b10`: show paths for Step5AF noise-bound + P2 glide experiment
- `deploy midi-v2b10`: rebuild/reflash Step5AF noise-bound + P2 glide experiment
- `status midi-v2b11`: show paths for Step5AG stream 5-lane + event experiment
- `deploy midi-v2b11`: rebuild/reflash Step5AG stream 5-lane + event experiment
- `status midi-v2b12`: show paths for Step5AH stream fast audio guard experiment
- `deploy midi-v2b12`: rebuild/reflash Step5AH stream fast audio guard experiment
- `status midi-v2b13`: show paths for Step5AI expressive FX + TRI glide experiment
- `deploy midi-v2b13`: rebuild/reflash Step5AI expressive FX + TRI glide experiment
- `status midi-v2b14`: show paths for Step5AJ poly/delay/release/anti-pop experiment
- `deploy midi-v2b14`: rebuild/reflash Step5AJ poly/delay/release/anti-pop experiment
- `status midi-v2b15`: show paths for Step5AK volume curve experiment
- `deploy midi-v2b15`: rebuild/reflash Step5AK volume curve experiment
- `status midi-v2b16`: show paths for Step5AL linear volume / wave / release experiment
- `deploy midi-v2b16`: rebuild/reflash Step5AL linear volume / wave / release experiment
- `status midi-v2b17`: show paths for Step5AM slider diag / wave / anti-pop experiment
- `deploy midi-v2b17`: rebuild/reflash Step5AM slider diag / wave / anti-pop experiment
- `status midi-v2b18`: show paths for Step5AN log slider compensation experiment
- `deploy midi-v2b18`: rebuild/reflash Step5AN log slider compensation experiment
- `status midi-v2b19`: show paths for Step5AO slider trim / long env experiment
- `deploy midi-v2b19`: rebuild/reflash Step5AO slider trim / long env experiment
- `status midi-v2b20`: show paths for Step5AP hi-zone slider experiment
- `deploy midi-v2b20`: rebuild/reflash Step5AP hi-zone slider experiment
- `status midi-v2b21`: show paths for Step5AQ longer env experiment
- `deploy midi-v2b21`: rebuild/reflash Step5AQ longer env experiment
- `status midi-v2b22`: show paths for Step5AR flat env / pitch cleanup experiment
- `deploy midi-v2b22`: rebuild/reflash Step5AR flat env / pitch cleanup experiment

Historical note:

- `midi` now points to the active build in `ROM V2B\v2b_step5ar_flat_env_pitch` + `Firmware Arduino\Firmware_PICO-V2B`
- archived step targets still work through the same short commands, but their source folders now live under `Archive\`

Local commands:

```powershell
.\tools\v2.cmd status midi
.\tools\v2.cmd build-rom midi
.\tools\v2.cmd push-rom midi
.\tools\v2.cmd build-pico midi
.\tools\v2.cmd flash-pico midi
.\tools\v2.cmd deploy midi
.\tools\v2.cmd status midi-v2a
.\tools\v2.cmd deploy midi-v2a
.\tools\v2.cmd status midi-v2b1
.\tools\v2.cmd deploy midi-v2b1
.\tools\v2.cmd status midi-v2b2
.\tools\v2.cmd deploy midi-v2b2
.\tools\v2.cmd status midi-v2b3
.\tools\v2.cmd deploy midi-v2b3
.\tools\v2.cmd status midi-v2b4
.\tools\v2.cmd deploy midi-v2b4
.\tools\v2.cmd status midi-v2b5
.\tools\v2.cmd deploy midi-v2b5
.\tools\v2.cmd status midi-v2b6
.\tools\v2.cmd deploy midi-v2b6
.\tools\v2.cmd status midi-v2b7
.\tools\v2.cmd deploy midi-v2b7
.\tools\v2.cmd status midi-v2b8
.\tools\v2.cmd deploy midi-v2b8
.\tools\v2.cmd status midi-v2b9
.\tools\v2.cmd deploy midi-v2b9
.\tools\v2.cmd status midi-v2b10
.\tools\v2.cmd deploy midi-v2b10
.\tools\v2.cmd status midi-v2b11
.\tools\v2.cmd deploy midi-v2b11
.\tools\v2.cmd status midi-v2b12
.\tools\v2.cmd deploy midi-v2b12
.\tools\v2.cmd status midi-v2b13
.\tools\v2.cmd deploy midi-v2b13
.\tools\v2.cmd status midi-v2b14
.\tools\v2.cmd deploy midi-v2b14
.\tools\v2.cmd status midi-v2b15
.\tools\v2.cmd deploy midi-v2b15
.\tools\v2.cmd status midi-v2b16
.\tools\v2.cmd deploy midi-v2b16
.\tools\v2.cmd status midi-v2b17
.\tools\v2.cmd deploy midi-v2b17
.\tools\v2.cmd status midi-v2b18
.\tools\v2.cmd deploy midi-v2b18
.\tools\v2.cmd status midi-v2b19
.\tools\v2.cmd deploy midi-v2b19
.\tools\v2.cmd status midi-v2b20
.\tools\v2.cmd deploy midi-v2b20
.\tools\v2.cmd status midi-v2b21
.\tools\v2.cmd deploy midi-v2b21
.\tools\v2.cmd status midi-v2b22
.\tools\v2.cmd deploy midi-v2b22
```

## Test Result Format

Preferred compact user format:

```text
midi silence KO log26
midi P1 OK
midi P2 absent
midi joue seul log26
midi Q20 OVF1
```

Expanded but still compact:

```text
target: midi
test: silence sans MIDI
result: KO, joue seul
log: C:\Users\mto1\Documents\NES_DEV\Log_26.txt
```

## Audio Keywords

- `silence`: no sound
- `joue seul`: sound without MIDI note input
- `retard`: stale/delayed notes
- `P1 ok`: pulse 1 audible and controlled
- `P2 ok`: pulse 2 audible and controlled
- `P1 absent`: pulse 1 missing
- `P2 absent`: pulse 2 missing
- `glide`: portamento heard
- `lfo`: modulation heard
- `no lfo`: no modulation heard

## Log Keywords

- `Q low`: queue healthy
- `Q20`: queue full
- `OVF0`: no overflow
- `OVF1`: overflow happened
- `SAFE`: 245 disabled/safe
- `ON`: 245 enabled
- `ACK`: NES is consuming events

## Assistant Response Style

Default response should be compact:

```text
Diag: ...
Action: ...
Need: ...
```

Avoid long recaps unless asked:
- no full history
- no large logs pasted back
- no re-explaining validated steps

## Decision Tokens

- `GO`: proceed with obvious next action
- `STOP`: pause
- `NEXT`: move to next feature
- `KEEP`: keep this version as current good base
- `ROLLBACK`: revert only the last experimental variant, not validated steps
- `PATCH`: modify code
- `FLASH`: flash Pico
- `ROM`: push ROM to `D:\game.nes`
- `LOG`: analyze referenced log
