# V2 Expansion Port - Pin Plan & Cablage Proto Minimal

Date de reference : 2026-05-08

## But

Sortir un bundle de signaux minimal pour :
- protocole `OUT0-OUT2 + D0-D4`
- option `/IRQ`
- sniff `D0-D7`
- reprise optionnelle de `R/W` et `M2` sur CPU pour instrumentation

V18 a valide `D0` comme voie live solide, mais a montre que `D3/D4` sur port manette ne sont pas fiables pour des controles riches sans degrader le groove. V2 se construit donc autour de l'expansion port.

## Note tres importante : "D0-D4" en V2 ne veut PAS dire "port manette 1 ou 2"

En V2, quand on parle de `D0-D4`, on parle des lignes **exposees sur l'expansion port**.

Donc :
- **V1** = ports manette facade / protocole controleur
- **V2** = expansion port sous la console

Le protocole propose utilise :
- ecriture `$4016` pour piloter `OUT0-OUT2`
- lecture `$4017` pour lire `D0-D4`

Cela ne veut pas dire "se brancher sur le port manette 2".
Cela veut dire :
- utiliser l'**expansion port**
- s'appuyer sur la voie de lecture associee a `$4017` / `/OE2`

## Schema visuel simplifie

```text
                NES CPU / ROM
                     |
        +------------+------------+
        |                         |
   write $4016                read $4017
        |                         |
     OUT0..OUT2               D0..D4 + /OE2
        |                         |
        +----------- Expansion Port -----------+
                                                |
                                             RP Pico
                                                |
                                   controles riches / UI / LFO / ADSR
```

Et surtout :

```text
V1 (ancien) :
Ports manette facade
  - D0 live stable
  - D3/D4 problematiques

V2 (nouveau) :
Expansion port
  - OUT0..OUT2 = commande
  - D0..D4 = data
  - /IRQ optionnel
  - D0..D7 bus CPU = sniff seulement
```

## A) Set minimal recommande

### Commande NES -> module
- `OUT0`
- `OUT1`
- `OUT2`

Ces lignes sont pilotees par les 3 bits bas de `$4016`.

### Donnee module -> NES
- `D0`
- `D1`
- `D2`
- `D3`
- `D4`

Ces lignes sont lues via `$4017` dans la proposition V2, avec `/OE2` comme strobe naturel de lecture.

### Alimentation
- `+5V`
- `GND`
- `GND` secondaire recommande dans la nappe

### Event optionnel
- `/IRQ`

## B) Sniff / instrumentation

### Bus data CPU
- `D0-D7` en lecture seulement

But :
- observer le bus CPU
- correler timing ROM / module
- accelerer le debug

Ne jamais driver ce bus dans le proto V2.

### Signaux CPU optionnels
- `R/W` repris directement sur CPU si besoin
- `M2` repris directement sur CPU si besoin

Ces deux lignes ne sont pas requises pour un premier protocole V2, mais elles sont tres utiles pour instrumentation.

## C) Traduction de niveaux

Recommandation de proto :
- `NES -> RP2040` : `74HC4050` pour descendre `5V -> 3.3V`
- `RP2040 -> NES` : logique `HCT` si le module doit piloter des lignes NES (`OUT`, `/IRQ`, etc.)

## D) Audio optionnel

Si la V2 veut aussi preparer un vrai mix externe :
- `AD1` = pulses
- `AD2` = triangle + noise + DMC

Ce point est separe du protocole de controle et doit rester optionnel dans le premier proto.

## E) Erreurs frequentes a eviter

- verifier l'orientation mecanique de l'expansion port au multimetre avant de souder toute la nappe
- ne pas driver `D0-D7` cote bus CPU
- ne pas supposer que `R/W` est disponible sur l'expansion port : si on le veut, il faut le reprendre ailleurs
- garder le proto V2 simple : `OUT0-OUT2`, `D0-D4`, `+5V`, `GND`, `/IRQ` optionnel

## F) Bundle proto 1 retenu

Pour le **proto 1 reellement retenu** avec une nappe `16 fils`, on ne part pas sur tout le bus CPU.

On garde :
- le protocole actif `OUT0-OUT2 + PORT1-0..4`
- le strobe de lecture `$4017` via `/OE2`
- un petit sniff `CPU D0..D2`
- `A15` comme bonus debug

Et on reporte :
- `/IRQ`
- `CPU D3..D7`
- `R/W`
- `M2`

### Liste exacte des 16 fils

1. `48` = `+5V`
2. `47` = `GND`
3. `02` = `GND`
4. `45` = `OUT2`
5. `44` = `OUT1`
6. `43` = `OUT0`
7. `11` = `/OE2`
8. `19` = `PORT1-0`
9. `20` = `PORT1-1`
10. `15` = `PORT1-2`
11. `16` = `PORT1-3`
12. `18` = `PORT1-4`
13. `05` = `A15`
14. `32` = `CPU D0`
15. `31` = `CPU D1`
16. `30` = `CPU D2`

### Etat du proto au moment de cette doc

Deja soudes :
- `48`, `47`
- `45`, `44`, `43`
- `19`, `20`, `15`, `16`, `18`

Reste a souder :
- `02`
- `11`
- `05`
- `32`
- `31`
- `30`

## G) Positionnement de cette doc

Cette doc ne decrit pas encore la logique firmware ou le protocole complet.
Elle sert a verrouiller :
- quelles lignes sortir
- lesquelles sont critiques
- lesquelles sont purement optionnelles

Voir aussi :
- [V2_EXPANSION_PORT_PROTOCOL_SPEC.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_PROTOCOL_SPEC.md)
- [V2_EXPANSION_PORT_TEST_PLAN.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_TEST_PLAN.md)
- [V2_GPIO_BUDGET_AND_CONTROLS.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_GPIO_BUDGET_AND_CONTROLS.md)
