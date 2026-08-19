# V2 Expansion Port Revalidation

Date: 2026-05-12

## Goal

Rebuild the **true physical mapping** of the NES expansion port **without using
any photo-based left/right interpretation**.

This procedure intentionally avoids spatial inference.

## User-confirmed orientation result

After revalidation, the user-confirmed solder-side orientation is:

```text
25 ........................................................................... 48
24 ........................................................................... 01
```

This result supersedes the old mirrored quick reference.

## Rules

- NES disconnected from all breadboards and proto wiring.
- Use a **known-good ground reference**, not a guessed expansion-port ground:
  - chassis metal tied to ground
  - RCA shield / AV ground
  - negative side of a known ground capacitor
- Use the multimeter in **V DC**.
- First identify `+5V` and `GND`.
- Only after that, identify `OUT0..OUT2`.

## Step 1 - Find real power pins

With the NES powered on and no proto attached:

1. Put black probe on known-good board/chassis ground.
2. Probe every expansion-port pin physically, one by one.
3. Record only:
   - `~0V`
   - `~+5V`
   - anything else

Expected truth from the official logical pinout:
- there are **two ground pins**
- there are **two +5V pins**

But this procedure does **not** assume where they are in the photo.

## Step 2 - Lock the real orientation

Once the two `+5V` pins and two `GND` pins are found:

1. Mark them physically on paper.
2. Compare that discovered pattern against the official logical pinout.
3. Only then determine whether the previous photo interpretation was mirrored.

Do not continue before this step is complete.

## Step 3 - Validate OUT lines with fixed ROMs

Use the three fixed ROMs:

- `V2_OUT0_FIX.nes`
- `V2_OUT1_FIX.nes`
- `V2_OUT2_FIX.nes`

Each ROM holds one output state:

- `OUT0` fixed -> expected bus value `001`
- `OUT1` fixed -> expected bus value `010`
- `OUT2` fixed -> expected bus value `100`

With black probe still on known-good ground:

1. Launch `V2_OUT0_FIX.nes`
2. Find which physical expansion-port pin goes high
3. Launch `V2_OUT1_FIX.nes`
4. Find which pin goes high
5. Launch `V2_OUT2_FIX.nes`
6. Find which pin goes high

This gives the **real physical OUT mapping**.

## Step 4 - Only then restore the ribbon map

After `+5V`, `GND`, and `OUT0..OUT2` are proven electrically:

1. Rewrite the ribbon color map from those facts.
2. Revalidate any already soldered wire by continuity.
3. Reconnect the proto one signal at a time.

## Recovery priority

Order of trust reconstruction:

1. Real `GND`
2. Real `+5V`
3. Real `OUT0`
4. Real `OUT1`
5. Real `OUT2`
6. Only later: `/OE2`
7. Only later: `A15`
8. Only later: CPU sniff lines

## Notes

- Previous continuity tests may still have been correct mechanically while the
  physical pin numbering assumption was wrong.
- Therefore continuity alone was not sufficient validation.
- The only trusted recovery path now is **electrical rediscovery first,
  documentation second**.
