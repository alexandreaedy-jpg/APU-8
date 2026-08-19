# V2 Expansion Port - Playbook Complet

Date de reference : 2026-05-08

## 0) Objectifs V2

On veut gagner par rapport a V18 :
- separation totale entre `notes / triggers` et `controles riches`
- ergonomie plus proche d'une midicart
- transport robuste et instrumentable
- iteration rapide grace au sniff du bus

## 0.1) Recentrage strategique

Avant d'aller vers une `V2_C` plus risquee, on epuisera volontairement les capacites de `V2_A / V2_B`.

Le pari principal n'est pas seulement :
- "plus de lignes"

Le vrai pari est aussi :
- `RP2040 Pico` beaucoup plus puissant que le Nano
- handshakes deterministes
- file d'evenements / FIFO
- `data ready`
- `ack`
- `sync`
- paquets atomiques
- meilleure priorisation entre `notes` et `controles`

Autrement dit :
- `V2_A / V2_B` ne sont pas seulement une etape prudente
- ce sont les **vraies versions a pousser au maximum** avant toute escalation bus-aware

La conclusion V18 est simple :
- `D0` port manette = voie live solide
- `D3/D4` port manette = trop couteux/fragiles pour des controles expressifs

Donc V2 ne tord plus le port manette. Elle deplace les controles riches vers l'expansion port.

## 1) Ce que la NES met a disposition

### Interface d'I/O retenue
- ecriture `$4016` : `OUT0-OUT2`
- lecture `$4017` : `D0-D4` avec `/OE2`

Ce duo suffit pour un premier protocole sans SRAM :
- `OUT0-OUT2` = commande / phase
- `D0-D4` = data / flags

### Pourquoi on s'eloigne du port manette

Le port manette impose :
- lecture active-low
- pulses de clock implicites a chaque read
- sous-ensemble de lignes utiles

V18 a montre que cette topologie etait bonne pour `D0`, mais pas pour des controles riches en parallele.

### Instrumentation

La V2 garde aussi une couche debug :
- `D0-D7` bus CPU en lecture seule
- `R/W` et `M2` si repris sur CPU

## 2) Architecture V2 recommandee

### Roles
- `ROM NES` : maitre du protocole
- `Module RP2040` : esclave intelligent, buffer, decode, renvoie et applique

### Separation des canaux
- `V1 / port manette` : notes, gates, triggers, groove live
- `V2 / expansion port` : ADSR, duty, LFO, macros, presets, UI riche

### Evenement optionnel
- `/IRQ` peut signaler `data ready`
- optionnel dans le premier proto

## 2.1) Hypothese forte de `V2_A / V2_B`

Le gain majeur attendu par rapport a `V1.7 / V1.8` peut venir :
- autant du `Pico + protocole`
- que du port lui-meme

Ce que `V2_A / V2_B` doivent prouver :
- qu'un transport **mieux discipline** peut suffire a piloter l'APU proprement
- meme sans saut radical de bande passante brute

Ce que `V2_A / V2_B` ne promettent pas encore :
- remplacer une vraie `V2_C2` bus-aware sur le plan theorique

Ce qu'elles peuvent en revanche apporter :
- moins de jitter pratique
- moins de bugs de framing
- meilleure stabilite quand les controles deviennent actifs
- meilleure coexistence `groove notes` + `controles`

## 3) Strategie de mise au point

### Phase 0
- valider les pins physiques
- verifier `OUT0-OUT2`
- verifier lecture stable de `D0-D4`
- verifier `A15`
- garder `CPU D0-D2` en bonus debug, sans en dependre

### Phase 1
- bring-up du protocole minimal
- `STATUS`
- `REG_ID`
- `VALUE_LO / VALUE_HI`
- `ACK_AND_NEXT`
- valider l'aller-retour NES <-> Pico sans glitch

### Phase 2
- implementer la file d'evenements cote Pico
- definir les priorites de service
- commencer par `DUTY`
- viser un premier comportement propre a `120 Hz`

### Phase 3
- ajouter `ADSR`
- ajouter `LFO rate/depth`
- tester les paquets atomiques et l'absence de regression groove

### Phase 4
- brancher la vraie UI
- stress test MIDI + controles
- instrumentation bus si besoin
- n'envisager `V2_B` etendue ou `V2_C` que si `V2_A` plafonne reellement

## 4) Doctrine projet

### Ce qu'on garde
- `V1` reste la reference live
- pas de regression groove acceptable sur `D0`

### Ce qu'on deplace
- les controles expressifs vont sur V2

### Ce qu'on s'autorise encore sur V1
- presets lents
- macros rares
- mode edition non-live

### Ce qu'on refuse pour l'instant
- sauter trop vite vers `V2_C`
- toucher `R/W` / `M2` avant d'avoir vraiment mesure les limites de `V2_A / V2_B`
- sous-estimer le gain potentiel du `Pico` par rapport au Nano

## 5) Conclusion

V2 n'est pas un abandon de V18.
V2 est la suite logique d'un constat experimental :
- `D0` est valide et musical
- les controles riches doivent sortir de la contrainte du port manette

Le cap corrige est maintenant clair :
- pousser `V2_A / V2_B` aussi loin que possible
- exploiter a fond les avantages du `Pico`
- ne considerer `V2_C` que si la limite est reellement atteinte

Voir aussi :
- [V2_AB_EXECUTION_PLAN.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_AB_EXECUTION_PLAN.md)

Voir aussi :
- [V2_EXPANSION_PORT_PIN_PLAN.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_PIN_PLAN.md)
- [V2_EXPANSION_PORT_PROTOCOL_SPEC.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_PROTOCOL_SPEC.md)
- [V2_EXPANSION_PORT_TEST_PLAN.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_TEST_PLAN.md)
