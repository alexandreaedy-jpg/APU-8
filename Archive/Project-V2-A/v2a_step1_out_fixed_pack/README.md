# V2A Step 1 - Fixed OUT Pack

Trois ROMs de diagnostic tres simples :

- `V2_OUT0_FIX.nes` force `$4016 = 0x01`
- `V2_OUT1_FIX.nes` force `$4016 = 0x02`
- `V2_OUT2_FIX.nes` force `$4016 = 0x04`

Chaque ROM joue aussi un son continu different pour aider a reconnaitre ce qui tourne, meme si l'affichage EverDrive est bugge.

But :
- tester une seule ligne `OUT` a la fois
- sortir du doute lie a la ROM one-hot et au menu EverDrive
