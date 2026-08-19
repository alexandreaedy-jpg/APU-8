## V2 Expansion Port Quick Reference

Date: 2026-05-04

### Orientation used here

This reference matches the photo orientation where:

- the **expansion port is at the top**
- the **72-pin cartridge connector is at the bottom**

In that exact view:

- reading starts at the **far left**
- first pair at left is `01 / 48`
- last pair at right is `24 / 25`

### Pin order in this photo orientation

```text
haut :  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24
bas  :  48  47  46  45  44  43  42  41  40  39  38  37  36  35  34  33  32  31  30  29  28  27  26  25
```

### V2 useful pins only

```text
haut : [01][02][03][04][05][06][07][08][09][10][11][12][13][14][15][16][17][18][19][20][21][22][23][24]
                                              |                           |
                                             A15                         /IRQ

bas  : [48][47][46][45][44][43][42][41][40][39][38][37][36][35][34][33][32][31][30][29][28][27][26][25]
        |   |       |   |   |                                           |   |   |   |   |   |   |   |
        5V  GND     O2  O1  O0                                          D0  D1  D2  D3  D4  D5  D6  D7
```

### Recommended proto 1 safe bundle

This is now the **recommended V2 proto 1 safe bundle**.

The idea is:
- keep the active protocol simple
- use `OUT0..OUT2` as command
- use `/OE2` plus the `PORT1-0..4` group as the read side for `$4017`
- keep only a **small sniff bundle**: `CPU D0`, `CPU D1`, `CPU D2`
- postpone `/IRQ` to a later step

```text
haut : [01][02][03][04][05][06][07][08][09][10][11][12][13][14][15][16][17][18][19][20][21][22][23][24]
            |           |                 |                     |   |   |
           GND        A15               OE2                  P1-2 P1-3 P1-4

bas  : [48][47][46][45][44][43][42][41][40][39][38][37][36][35][34][33][32][31][30][29][28][27][26][25]
        |   |       |   |   |                       |   |   |
        5V  GND     O2  O1  O0                    CD0 CD1 CD2
```

Additional lines on the same safe proto bundle:

```text
`19 = PORT1-0`
`20 = PORT1-1`
```

So the **safe first-choice wiring set** is:

1. `48` = `+5V`
2. `47` = `GND`
3. `02` = `GND` secondaire
4. `45` = `OUT2`
5. `44` = `OUT1`
6. `43` = `OUT0`
7. `11` = `/OE2` (`$4017` read strobe)
8. `19` = `PORT1-0`
9. `20` = `PORT1-1`
10. `15` = `PORT1-2`
11. `16` = `PORT1-3`
12. `18` = `PORT1-4`
13. `05` = `A15` bonus debug
14. `32` = `CPU D0` sniff
15. `31` = `CPU D1` sniff
16. `30` = `CPU D2` sniff

### What is already soldered in the current proto

- `48` = `+5V`
- `47` = `GND`
- `45` = `OUT2`
- `44` = `OUT1`
- `43` = `OUT0`
- `19` = `PORT1-0`
- `20` = `PORT1-1`
- `15` = `PORT1-2`
- `16` = `PORT1-3`
- `18` = `PORT1-4`

### What remains to solder now

1. `02` = `GND` secondaire
2. `11` = `/OE2`
3. `05` = `A15`
4. `32` = `CPU D0` sniff
5. `31` = `CPU D1` sniff
6. `30` = `CPU D2` sniff

### Current color-coded ribbon order

The correct mental model is:
- this is a **16-wire ordered bundle**
- colors form a repeated sequence
- the safe way to read it is **by ribbon order**, not "one color = one unique signal"

Ribbon order currently understood from the latest diagram:

1. violet = `02` = `GND2`
2. grey = `30` = `CPU D2`
3. white = `31` = `CPU D1`
4. black = `32` = `CPU D0`
5. brown = `11` = `/OE2`
6. red = `05` = `A15`
7. orange = `18` = `Joypad D4`
8. yellow = `16` = `Joypad D3`
9. green = `15` = `Joypad D2`
10. blue = `20` = `Joypad D1`
11. violet = `19` = `Joypad D0`
12. grey = `43` = `OUT0`
13. white = `44` = `OUT1`
14. black = `45` = `OUT2`
15. brown = `47` = `GND`
16. red = `48` = `+5V`

Important:
- `Joypad D0..D4` here corresponds to the five data lines already soldered:
  - `19`, `20`, `15`, `16`, `18`
- the repeated colors are normal in this scheme
- what matters is the **position in the ribbon**

### Advanced bundle (more powerful, more risky)

This is the other direction we discussed, but **not** the recommended bring-up path:
- `CPU D0..D7`
- `A15`
- `/IRQ`
- later `R/W` and `M2`

Keep this for a later bus-aware proto, not for proto 1.

### Signal list

#### Top row

- `02` = `GND`
- `05` = `A15`
- `11` = `/OE2` (`$4017` read strobe)
- `15` = `PORT1-2`
- `16` = `PORT1-3`
- `18` = `PORT1-4`
- `19` = `PORT1-0`
- `20` = `PORT1-1`

#### Bottom row

- `48` = `+5V`
- `47` = `GND`
- `45` = `OUT2`
- `44` = `OUT1`
- `43` = `OUT0`
- `32` = `CPU D0`
- `31` = `CPU D1`
- `30` = `CPU D2`
- `29` = `CPU D3`
- `28` = `CPU D4`
- `27` = `CPU D5`
- `26` = `CPU D6`
- `25` = `CPU D7`

### Quick reality check before soldering all lines

- `02` and `47` should both be `GND`
- `43`, `44`, `45` should be the three contiguous `OUT0`, `OUT1`, `OUT2` lines
- in this **proto 1 safe** version, `11` is the important read strobe line for `$4017`
- `19`, `20`, `15`, `16`, `18` are the five controller data lines we are actively using here
- `/IRQ` is intentionally **not** part of this first `16-wire` bundle

If that matches your meter, the orientation is locked.

### Cable / connector note

For a first V2 bundle, we need roughly `16` lines.

What exists in standard parts:

- `16-conductor ribbon cable`
- `HE10 / IDC 16-pin connectors`
- `16-pin IDC board headers`

What probably does **not** exist as a neat off-the-shelf "perfect V2 cable":

- a ready-made round cable already terminated exactly for this project

So the realistic options are:

1. flat `16-way ribbon` with IDC/HE10 ends
2. ribbon cable that is separated for a short distance near the motherboard
3. custom round multicore cable only if we later want a cleaner final build

### Sources

- https://www.nesdev.org/wiki/Expansion_port
- https://commons.wikimedia.org/wiki/File:Nintendo-NES-Mk1-Motherboard-Bottom.jpg
- https://www.e44.com/connectique/connecteurs/fiches/IDC-16b/Femelle/he10-femelle-sertir-16-pins-HE10F16.html
- https://www.e44.com/connectique/connecteurs/fiches/DIN-3-broches/Male/he10-male-sertir-16-pins-pas-2.54mm-HE10M16.html
- https://www.e44.com/connectique/connecteurs/embases-ci/IDC-16b/Male/he10-male-droit-16-pins-HE10MD16.html
- https://www.e44.com/cablages/cables/cables-en-nappe/cable-en-nappe-16-conducteurs-0.08mm2-pas-1.27mm-1m-gris-FNG16-1.html
- https://www.e44.com/cablages/cables/cables-en-nappe/
