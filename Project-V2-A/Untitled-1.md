# NES MIDI Module – Controller Port (V1.x) | README V2 (technique / Codex-ready)

> **But** : obtenir un *trigger MIDI quasi parfait* via le port manette NES (D0/D3/D4), **sans DMC (DPCM)**.
>
> Motivation : le DMC lit des samples via DMA et peut (1) voler des cycles CPU (stall) et (2) corrompre certaines lectures (joypads/PPUDATA) selon le timing, ce qui détruit la déterminisme nécessaire à un groove parfait. [1](https://www.nesdev.org/wiki/DMA)[2](https://www.nesdev.org/wiki/Controller_Reading)

---

## 0) Résumé exécutif

Le port manette devient un **bus série synchrone piloté par le CPU** :

- La ROM pilote le **LATCH** via écriture dans `$4016`, puis lit `$4017` en rafale : **chaque read envoie un pulse CLK** et lit 1 bit sur D0..D4. [2](https://www.nesdev.org/wiki/Controller_Reading)[3](https://www.nesdev.org/wiki/Standard_controller)
- Le module (Nano/RP Pico plus tard) prépare les données **en avance**, publie un état stable sur les entrées parallèles du 4021 et laisse le CPU NES clocker le shift via ses reads.

Objectif : **latence déterministe** (= “jusqu’au prochain LATCH”) + jitter perceptible ≈ 0.

---

## 1) Faits matériels à connaître (NES + 4021)

### 1.1. LATCH + CLK : ce que fait vraiment la NES
- Pour les contrôleurs standard : on fait typiquement `write 1` puis `write 0` sur `$4016` afin de figer l’état, puis on lit 8 fois `$4016` ou `$4017` pour récupérer les bits. [2](https://www.nesdev.org/wiki/Controller_Reading)[3](https://www.nesdev.org/wiki/Standard_controller)
- Lire `$4016/$4017` **envoie un pulse de clock** sur la ligne CLK du port contrôleur (et lit 1 bit à chaque read). [2](https://www.nesdev.org/wiki/Controller_Reading)[3](https://www.nesdev.org/wiki/Standard_controller)
- La lecture est **active-low** : un niveau haut se lit comme 0, un niveau bas se lit comme 1. [2](https://www.nesdev.org/wiki/Controller_Reading)[4](https://www.nesdev.org/wiki/Controller_port_pinout)

**Conséquence** : ton transport est intrinsèquement “CPU-clocked”. C’est une force : le timing devient déterministe si la ROM l’est.

### 1.2. 4021 : ta “couche physique” réelle
Le 4021 est un registre PISO (Parallel-In Serial-Out) : il capture 8 entrées parallèles quand le latch est actif, puis les sort en série à chaque clock. [5](https://www.nesdev.org/wiki/4021)

Point crucial :
- Sur NES standard, la NES lit la sortie série du 4021 via **D0 ← Q8 (pin 3)**. [5](https://www.nesdev.org/wiki/4021)
- Le latch du 4021 est piloté par l’écriture `$4016 bit0` (strobe). [5](https://www.nesdev.org/wiki/4021)[3](https://www.nesdev.org/wiki/Standard_controller)

**Conséquence** :
- Le **premier bit** que la NES observe juste après le LATCH correspond à l’étage **PI-8 / Q8**. [5](https://www.nesdev.org/wiki/4021)

---

## 2) Problème #1 (le plus fréquent) : bit-order (reverse)

### 2.1. Pourquoi ça casse “enveloppe / duty”
Les contrôles (D3) et LFO (D4) sont des paquets avec **headers + checksum**. Si l’ordre des bits est inversé, les headers ne matchent plus, donc la ROM n’applique jamais ADSR/duty.

### 2.2. Deux solutions — choisir UNE et verrouiller

#### Solution A (recommandée) : câblage qui évite le reverse
Comme PI-8 sort en premier via Q8, le mapping simple pour sortir bit0 en premier est :

- 595 Q0 → 4021 PI-8
- 595 Q1 → 4021 PI-7
- …
- 595 Q7 → 4021 PI-1

=> plus besoin de `reverseByte()`. [5](https://www.nesdev.org/wiki/4021)

#### Solution B : reverse logiciel
Si ton câblage est “naturel” (Q0→PI-1 … Q7→PI-8), alors bit7 sort en premier :
- soit tu `reverseByte()` côté module sur l’octet publié,
- soit tu corriges côté ROM.

=> **faire le reverse au plus près du 595** est généralement le plus simple (recommandation).

---

## 3) Problème #2 : inversion active-low

Le bus contrôleur est inversé (HIGH->0, LOW->1). [2](https://www.nesdev.org/wiki/Controller_Reading)[4](https://www.nesdev.org/wiki/Controller_port_pinout)

**Règle d’or** : corriger l’inversion **une seule fois** (ROM ou module, pas les deux).

---

## 4) Pipeline “1-frame ahead” (zéro jitter perceptible)

### 4.1. Le principe
La seule latence acceptable est : “attendre le prochain LATCH”.
Donc on garantit :
> les données de la frame N+1 sont prêtes AVANT le LATCH de N+1.

### 4.2. Double-buffer par lane
Pour chaque lane (D0/D3/D4) :
- `active` = état calculé (MIDI, pots, arp…)
- `latched` = état qui sera capturé par le 4021 au prochain LATCH

### 4.3. Fenêtre de commit (moment sûr)
Fenêtre recommandée :
> **juste après le front descendant du LATCH (LATCH↓)**

Pourquoi :
- le 4021 vient de capturer la frame courante
- tu as le maximum de temps avant le prochain LATCH

### 4.4. Commit atomique multi-lanes
Objectif : éviter “pollution” (D0 de N+1 + D3 de N).

**Règle** :
- copie `activeD0/D3/D4 → latchedD0/D3/D4` dans une section critique (interrupts OFF)
- puis publie sur les entrées parallèles (via 595) de manière cohérente

---

## 5) ROM : lecture déterministe (cadence fixe)

### 5.1. Pattern de lecture standard
Le protocole standard de lecture contrôleur :
1) `write $4016 = 1` (LATCH HIGH)
2) petit délai
3) `write $4016 = 0` (LATCH LOW)
4) `read $4017` × 8 (shift)
Chaque read provoque un pulse CLK. [2](https://www.nesdev.org/wiki/Controller_Reading)[3](https://www.nesdev.org/wiki/Standard_controller)

### 5.2. Recommandation : “cycles fixes” sur les rafales
(Recommandation)
- éviter les boucles avec branches variables pendant le burst critique
- préférer un “unroll” des 8 reads ou une routine au timing constant

---

## 6) Stratégie fast lane / slow lanes

### D0 = voie rapide
- notes / triggers / arp
- polling fréquent
- paquet court + checksum

### D3/D4 = voies lentes (params)
- ADSR/duty (D3), LFO (D4)
- envoi rare
- header+checksum systématiques
- côté ROM : appliquer seulement si header OK (+ éventuellement “stable count” à la manière de ta D4) (recommandation)

---

## 7) Exclure le DMC (DPCM) dans V1.x

Pourquoi :
- Le DMC lit des bytes via DMA, peut staller le CPU et introduire des bugs/collisions de lecture (joypads/PPUDATA). [1](https://www.nesdev.org/wiki/DMA)[2](https://www.nesdev.org/wiki/Controller_Reading)
- Pour un groove chirurgical, on veut éviter toute source de non-déterminisme.

Donc :
- V1.x “port manette live” = **DMC OFF**
- DMC réservé à une future archi plus robuste (expansion/cart)

---

## 8) Schéma timing (ASCII)

ROM (NES) :

$4016=1  ────────────────┐  LATCH HIGH (capture 4021)
│
$4016=0  ────────┐       └─ LATCH LOW
│
READ $4017 x8:    R R R R R R R R   (chaque read = pulse CLK)
(LATCH et reads décrits par NESdev) [2](https://www.nesdev.org/wiki/Controller_Reading)[3](https://www.nesdev.org/wiki/Standard_controller)

Module (Nano) :

Après LATCH↓ : commit active→latched + précharge 595
Pendant READ burst : ne touche à rien

---

## 9) Implémentation firmware – recommandations avancées (Codex)

> Ceci est une section “recommandations techniques” (pas une citation hardware).
> But : réduire variance temporelle et éviter de rater une fenêtre de commit.

### 9.1. ISR minimaliste
- ISR = détecte LATCH↓, pose `commit_pending = 1`
- Main loop = fait le commit + précharge
But : éviter que l’ISR soit interrompue par UART/ADC, et éviter le jitter induit.

### 9.2. Précharge 595 + latch STCP
- Shifter (SHCP) peut être fait n’importe quand (hors burst)
- STCP (latch) est fait dans la fenêtre sûre

### 9.3. SPI hardware pour 595
- Utiliser SPI réduit la durée et la variance du shift (temps constant)
- Permet un commit plus court, donc plus de marge avant LATCH

### 9.4. Commit “groupé”
- 1 commit = copie des buffers + publication des 595 + fin
- jamais publier D3 puis D0 dans deux instants différents

---

## 10) Checklist debug (quand “ADSR/Duty ne reviennent pas”)

1) Header D3 : la ROM doit voir `0xD3 0x3D` → sinon bit-order/reverse. [2](https://www.nesdev.org/wiki/Controller_Reading)[3](https://www.nesdev.org/wiki/Standard_controller)
2) Active-low : valeurs à l’envers → inversion corrigée au mauvais endroit. [2](https://www.nesdev.org/wiki/Controller_Reading)[4](https://www.nesdev.org/wiki/Controller_port_pinout)
3) Checksum XOR : doit passer.
4) Fenêtre commit : si tu publies pendant burst, header instable.

---

## 11) Notes “vers la V2 (expansion/cart)”
- Garder ce protocole “CPU-clocked” (excellent)
- Réintroduire DMC seulement sur une archi où l’I/O ne dépend pas des reads joypad
- Exploiter OUT0..OUT2 et /OE (si expansion) selon la doc “Input devices”. [6](https://www.nesdev.org/wiki/Input_devices)

---

## Références clés
- Controller reading (LATCH, read→clock pulse, active-low) : https://www.nesdev.org/wiki/Controller_Reading [2](https://www.nesdev.org/wiki/Controller_Reading)
- Standard controller (séquence 1/0 puis 8 reads, ordre du report) : https://www.nesdev.org/wiki/Standard_controller [3](https://www.nesdev.org/wiki/Standard_controller)
- 4021 (D0 ← Q8, pinout, latch/clock) : https://www.nesdev.org/wiki/4021 [5](https://www.nesdev.org/wiki/4021)
- DMA / DMC DMA (stall + corruptions possibles) : https://www.nesdev.org/wiki/DMA [1](https://www.nesdev.org/wiki/DMA)
- Input devices (/OE, OUT0..OUT2, lignes dispo) : https://www.nesdev.org/wiki/Input_devices [6](https://www.nesdev.org/wiki/Input_devices)


Bonus : “mini plan d’action” (hyper concret) pour toi
Sans te poser 15 questions, voilà l’ordre d’attaque optimal :

Verrouiller bit-order D3 : le header 0xD3 0x3D doit être vu à 100%. [nesdev.org], [nesdev.org]
Verrouiller l’inversion (active-low) à un seul endroit. [nesdev.org], [nesdev.org]
Ajouter (si besoin) un “stable count” simple pour D3 comme tu le fais sur D4 (pour éviter un apply sur un paquet rare glitché). (Recommandation, basée sur ton approche D4)
DMC OFF sur cette branche live, car DMA DMC peut perturber CPU + lectures. [nesdev.org], [nesdev.org]
Ensuite, seulement ensuite : optimisation ISR/SPI.


