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

### Signal list

#### Top row

- `05` = `A15`
- `14` = `/IRQ` with `1 kOhm` series resistor

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

- `01` and `48` should both be `+5V`
- `02` and `47` should both be `GND`

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
