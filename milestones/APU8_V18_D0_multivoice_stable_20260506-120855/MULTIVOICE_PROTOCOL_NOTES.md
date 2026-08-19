# V18 Multivoice Notes

Base de départ:
- sketch stable figé: `arduino/NanoNesV18DN4/build-final-normal`
- source stable réaligné: `arduino/NanoNesV18DN4/NanoNesV18DN4.ino`
- copie de travail sketch: `arduino/NanoNesV18DN4_multivoice/NanoNesV18DN4_multivoice.ino`
- copie de travail ROM: `project-v18-DN4_multivoice`

Etat matériel validé:
- mode `P1` sain et réactif
- `D12` et `D13` débranchés du Nano dans l'état stable
- `LATCH` et `CLK` restent branchés vers le `4021`

Pièges déjà confirmés:
- `D13` branché au `CLK` NES pollue la ligne et casse la stabilité
- un flux `D0` circulaire qui cherche un header `0xA8 0x58` dans les données est ambigu
- l'ancien essai multi-voix pouvait se resynchroniser au mauvais endroit et produire:
  - fausses hauteurs
  - pseudo-arpèges
  - triggers ratés

Contraintes pour la prochaine version:
- ne pas dépendre de `D13`
- ne pas casser l'état `P1` stable dans `arduino/NanoNesV18DN4`
- travailler uniquement dans les copies `*_multivoice`
- utiliser un framing déterministe, pas une recherche libre de header dans un flux continu

Direction recommandée:
- avancer la trame sur un événement de `LATCH` seulement, pas sur un comptage logiciel des clocks
- réserver des octets de synchronisation impossibles dans les données utiles
- ou utiliser un index/phase explicite qui permet à la ROM de savoir quel octet elle lit

Essai chargé le 2026-05-06:
- transport `D0` compacté de `14` à `9` octets
- plus de lecture bloquante de trame complète côté ROM
- parser incrémental côté NES: `1` octet lu par poll, validation par header + checksums
- triggers des 4 voix portés par un masque de toggles, plus `1` octet dédié par trigger
- câblage visé pour ce build: `NES OUT/LATCH -> Nano D12`, aucun `NES CLK -> Nano D13`
