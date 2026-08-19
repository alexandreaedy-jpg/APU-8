# V2 Proto 1 - Breadboard Architecture (Pico Side)

Date de reference : 2026-05-08

## But

Definir une architecture physique simple et robuste pour le **proto 1 V2** avec une separation claire :
- **petite breadboard** = `RP2040 Pico + opto MIDI`
- **grande breadboard** = toute la **communication NES**

Sans melanger :
- le bus actif utile au bring-up
- le sniff optionnel
- les extensions futures

Cette doc ne remplace pas le pin plan. Elle dit **ou poser quoi** sur la breadboard et **dans quel ordre** valider.

## Rappel du bundle proto 1 retenu

### Actif
- `OUT0`
- `OUT1`
- `OUT2`
- `/OE2`
- `Joypad D0`
- `Joypad D1`
- `Joypad D2`
- `Joypad D3`
- `Joypad D4`

### Bonus debug garde des le proto 1
- `A15`
- `CPU D0`
- `CPU D1`
- `CPU D2`

### Alim
- `+5V`
- `GND`
- `GND2`

## Principe electrique

### Sens NES -> Pico

Les signaux NES `5V` vers `RP2040 3.3V` passent par des `74HC4050` alimentes en `3.3V`.

Lignes concernees :
- `OUT0`
- `OUT1`
- `OUT2`
- `/OE2`
- `A15`
- `CPU D0`
- `CPU D1`
- `CPU D2`

### Sens Pico -> NES

Les signaux du `RP2040` qui doivent piloter la NES passent par un `74HCT245` alimente en `5V`.

Lignes concernees :
- `Joypad D0`
- `Joypad D1`
- `Joypad D2`
- `Joypad D3`
- `Joypad D4`

## Architecture physique recommandee

### Breadboard 1 - petite board "controleur"

Cette breadboard reste dediee a :
- `RP2040 Pico`
- `opto MIDI`

Regle de base :
- `opto` alimente par le `+5V NES`
- `Pico` alimente par `USB`
- masse commune obligatoire entre NES, opto et Pico

Cette petite board ne porte pas la logique de bus NES. Elle sert surtout de coeur de calcul / MIDI.

### Breadboard 2 - grande board "bus NES"

La grande breadboard porte toute la communication avec la NES :
- arrivee de la nappe expansion port
- `74HC4050 #1`
- `74HC4050 #2` optionnel
- `74HCT245`
- decouplage
- eventuels straps de controle

Le Pico peut rester sur la petite board, avec des jumpers allant vers cette grande board.

## Variante minimale vs variante etendue

### Variante minimale realiste

Avec **1 seul `74HC4050`**, on peut deja faire le premier bring-up utile en gardant :
- `OUT0`
- `OUT1`
- `OUT2`
- `/OE2`
- `A15`

Ca fait `5` canaux sur `6`, donc ca rentre dans un seul boitier.

Dans cette variante :
- on reporte le sniff `CPU D0..D2`
- on garde quand meme le `74HCT245` pour `Joypad D0..D4`

### Variante etendue proto 1

Avec **2 x `74HC4050`** :
- le premier reste dedie aux lignes actives
- le second sert au sniff `CPU D0..D2` et garde de la marge pour plus tard

Donc :
- `2 x 4050` = version confortable / evolutive
- `1 x 4050` = version minimale parfaitement valable pour demarrer

## Zoning concret

### Zone A - Arrivee nappe NES

Regrouper les 16 fils de nappe sur un bord de la breadboard, dans l'ordre de la nappe.

Objectif :
- minimiser les croisements
- garder la lecture visuelle facile
- pouvoir sonder rapidement au multimètre ou au scope

### Zone B - `74HC4050 #1` : bus actif NES -> Pico

Dedier le premier `4050` uniquement aux lignes actives du proto 1 :
- `OUT0`
- `OUT1`
- `OUT2`
- `/OE2`
- `A15`

Il reste 1 canal libre.

Pourquoi :
- ce bloc devient la zone "commande / timing"
- plus facile a debug que de melanger avec le sniff CPU

### Zone C - `74HC4050 #2` : sniff debug

Dedier le second `4050` au sniff :
- `CPU D0`
- `CPU D1`
- `CPU D2`

Les autres canaux restent libres pour plus tard :
- `CPU D3..D5` ou `R/W`, `M2`

Pourquoi :
- on garde le sniff physiquement separe du protocole actif
- si le sniff cree du doute, on peut le debrancher sans toucher au coeur V2

### Zone D - `74HCT245` : sorties Pico -> NES

Dedier le `245` uniquement aux 5 lignes `Joypad D0..D4`.

Les 3 voies restantes peuvent rester non cablees pour le proto 1.

Important :
- choisir un cote du `245` pour le Pico
- choisir l'autre cote pour la NES
- fixer ensuite la direction une fois ce choix etabli

Recommendation pratique :
- `A-side` = Pico
- `B-side` = NES

Dans ce cas :
- `DIR` strap haut pour `A -> B`
- `OE` garde **desactive par defaut**

## Strategie safe pour le `74HCT245`

Le vrai piege du proto 1, c'est de **driver la NES trop tot**.

Donc :
- `DIR` peut etre strap en dur une fois le sens choisi
- `OE` ne doit pas etre force actif en permanence des le premier allumage

Recommendation :
- `OE` avec pull-up `10k` vers `+5V`
- donc `245` **desactive par defaut**
- un GPIO Pico pourra tirer `OE` a `0` plus tard quand le firmware est pret

Avantage :
- pas de drive parasite au boot du Pico
- bring-up plus serein

## Alimentation

### Rail 5V

Sur la **grande breadboard**, utiliser le `+5V` NES pour :
- `74HCT245`
- pull-up / logique 5V associee
- et, si tu veux simplifier le cablage global, les `74HC4050`

### Rail 3.3V

Le `3.3V` du Pico doit au minimum alimenter :
- les GPIO du Pico
- les eventuelles references logiques cote petite breadboard

Pour le proto 1, le plus simple est :
- `Pico` sur `USB`
- logique NES de la grande board sur `+5V NES`

Important :
- si tu alimentes les `74HC4050` en `5V`, ils **ne level-shiftent pas tout seuls**
- il faut alors verifier soigneusement ce qui arrive au Pico
- pour rester simple et safe, garde l'idee initiale : **4050 alimentes en 3.3V** si ce sont eux qui protegent les entrees Pico

Donc, en pratique :
- petite board : `Pico USB`
- grande board : `245` en `5V NES`
- `4050` en `3.3V` amene depuis le Pico vers la grande board

### Masse

Tout doit partager la meme masse :
- `NES GND`
- `NES GND2`
- `Pico GND`
- `4050 GND`
- `245 GND`
- `opto GND`

### Recommandation de bring-up

Pour le premier allumage :
- alimenter le Pico par `USB`
- alimenter l'opto par `+5V NES`
- garder la masse commune avec la NES
- ne pas compter sur le `+5V` NES pour alimenter le Pico

## Decouplage

Ajouter des `100 nF` au plus pres :
- `74HC4050 #1`
- `74HC4050 #2`
- `74HCT245`

Ajouter aussi :
- `47 uF` a `100 uF` entre `+5V` et `GND`
- `10 uF` environ entre `3.3V` et `GND` cote Pico / buffers si utile

## Ordre de validation recommande

### Etape 1 - power only
- petite board : `Pico + opto`
- grande board : rails `5V`, `3.3V`, `GND`
- `4050` et `245` poses
- decouplage en place

### Etape 2 - lecture NES -> Pico only
- cabler `OUT0`
- `OUT1`
- `OUT2`
- `/OE2`
- pas encore de `245` actif
- Pico relie a la grande board uniquement pour les lignes de lecture

Objectif :
- verifier que le Pico voit bien les ecritures `$4016` et les lectures `$4017`

### Etape 3 - ajouter `A15` puis sniff `CPU D0..D2`
- `A15` peut deja etre present dans la variante `1 x 4050`
- le sniff `CPU D0..D2` demande le second `4050` si on veut garder le cablage propre

### Etape 4 - activer les sorties `Joypad D0..D4`
- cabler `245`
- garder `OE` inhibe par defaut
- premier test avec pattern fixe

## Ce qu'il ne faut pas faire

- ne pas melanger `OUT0..OUT2` et `CPU D0..D2` sur le meme `4050` sans raison
- ne pas activer le `245` des le boot sans controle de `OE`
- ne pas alimenter les `4050` en `5V` si leur role est de proteger le Pico
- ne pas faire passer toute la nappe d'abord dans le Pico puis revenir vers les buffers
- ne pas essayer de faire porter a la petite breadboard la logique de bus NES

## Verdict net

Pour le proto 1, l'architecture saine est :
- petite breadboard = `Pico + opto`
- grande breadboard = `1 x 74HC4050` minimum + `1 x 74HCT245` + nappe NES
- `2 x 74HC4050` si on active aussi le sniff `CPU D0..D2`
- `245` dedie aux `Joypad D0..D4`
- `OE` du `245` desactive par defaut
- `Pico` alimente par `USB`
- `opto` alimente par `+5V NES`
- masse commune partout

Suite logique :
- faire ensuite le **cablage pin par pin** `Pico + 4050 + 245`
