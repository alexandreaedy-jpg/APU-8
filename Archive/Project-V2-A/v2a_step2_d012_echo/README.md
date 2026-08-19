# V2 Step 2 - D0/D1/D2 echo

This ROM is a pure readback diagnostic.

## Goal

- unlock the Pico-side `74HCT245`
- read `$4017 & 0x07`
- write that 3-bit value back to `$4016`

Because the `OUT0..OUT2` path is already validated, the Pico can then show
exactly what the NES is reading on `D0..D2`.

## Expected with the current Step 2 Pico firmware

As the Pico drives:
- `D=00001`
- `D=00010`
- `D=00100`
- `D=01000`
- `D=10000`
- `D=00000`

the observed `OUT` should become:
- `100`
- `010`
- `001`
- `000`
- `000`
- `000`

If the `OUT` echo does not match that pattern, the remaining fault is on the
`245 -> expansion port -> $4017` readback side.
