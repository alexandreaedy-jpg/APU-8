# V2 Expansion Port - Budget GPIO et Controles

Date de reference : 2026-05-04

Objectif :
- verifier si une V2 expansion port sur RP2040 peut gerer a la fois :
  - le bus NES critique
  - 8 pots
  - 8 selecteurs rotatifs
  - 1 vrai switch

Conclusion courte :
- oui, c'est faisable sur un Pico / RP2040
- mais pas si on branche tous les controles en direct
- il faut multiplexer les controles

## Hypothese de base

On cherche une V2 raisonnable, pas une carte finale maximaliste des le premier proto.

On veut garder une marge pour :
- l'audio / timing
- les controles physiques
- la re-utilisation d'une partie du materiel V18 si besoin

## Ressource de base du RP2040 / Pico

Sur un Pico standard, on retient :
- 26 GPIO utilisateur
- 3 ADC exposes de facon simple pour l'utilisateur

Ca suffit pour la V2 si on fait une architecture propre.

## Budget du bus V2

### Version bus minimale realiste

1. Lecture du bus NES vers RP2040
- CPU D0..D7 = 8 GPIO entree

2. Ecriture RP2040 vers bus NES
- D0..D7 retour vers 74HCT245 = 8 GPIO sortie

3. Lignes de controle / observation
- A15 = 1 GPIO entree
- OUT0 = 1 GPIO entree

Total bus minimal :
- 18 GPIO

### Version bus minimale + un peu de marge protocolaire

Ajouter au besoin :
- R/W = 1 GPIO entree

Total bus etendu :
- 19 GPIO

### Ce qui ne mange pas forcement de GPIO RP2040

Si on pense bien la logique autour :
- 74HCT245 : DIR peut etre strap en dur
- 74HCT245 : OE peut etre gere par la logique externe
- 74HCT138 : decode adresse, pas besoin d'etre pilote par le RP2040
- 74HCT573 : peut etre utilise avec un controle tres simple

Donc le vrai budget RP2040 du bus reste plus bas qu'on pourrait le craindre.

## Architecture controle recommandee

### 8 pots

Solution recommandee :
- 1 x 4051 analogique

Budget GPIO :
- 1 ADC
- 3 GPIO de selection communs

Total :
- 4 GPIO

### 8 selecteurs rotatifs

Solution recommandee si ce sont de vrais selecteurs de mode :
- 1 x second 4051
- chaque selecteur rotatif produit une tension discrete via un petit reseau de resistances
- lecture par ADC avec fenetres de seuil

Tres gros avantage :
- les deux 4051 peuvent partager les 3 lignes de selection

Budget GPIO additionnel :
- 1 ADC supplementaire
- 0 GPIO de selection supplementaire

Total pour 8 pots + 8 rotatifs :
- 2 ADC
- 3 GPIO de selection
- soit 5 GPIO seulement

### 1 vrai switch

Solution simple :
- 1 GPIO direct

## Budget total recommande

### Variante A - recommandee

Bus minimal :
- 18 GPIO

Controles :
- 8 pots via 4051 = 4 GPIO
- 8 rotatifs via second 4051 partage = 1 GPIO ADC supplementaire
- 1 vrai switch = 1 GPIO

Total controles :
- 6 GPIO

Total general :
- 24 GPIO

Si on ajoute R/W :
- 25 GPIO

Verdict :
- ca passe sur RP2040
- avec une petite marge restante

## Variante B - re-utilisation du materiel V18

Si on veut recycler le plus possible l'experience V18 :

### 8 pots
- 1 x 4051
- cout = 4 GPIO

### 8 rotatifs + 1 switch

Si les selecteurs peuvent etre lus comme signaux digitaux discrets :
- 2 x 4021 chaines
- ou 2 x 74HC165

Budget GPIO :
- DATA = 1
- CLOCK = 1
- LATCH = 1

Total :
- 3 GPIO pour 16 bits dispo

Donc :
- 8 rotatifs + 1 switch = 3 GPIO

### Total variante B

Bus minimal :
- 18 GPIO

Controles :
- 8 pots via 4051 = 4 GPIO
- 8 rotatifs + 1 switch via 4021 / 165 = 3 GPIO

Total general :
- 25 GPIO

Avec R/W :
- 26 GPIO

Verdict :
- ca passe encore
- mais on devient serre
- cette variante est interessante surtout si on veut re-utiliser du materiel V18

## Ce qu'il ne faut pas faire

### Tout brancher en direct

Exemple absurde mais utile :
- 8 pots directs = 8 GPIO analogiques
- 8 selecteurs directs = 8 GPIO
- 1 switch direct = 1 GPIO

Total controles seuls :
- 17 GPIO

Ca tuerait toute la marge pour le bus.

Donc :
- non, pas de panneau "un fil par controle"

## Recommandation finale

### Pour une V2 saine

Je recommande :
- bus V2 minimal = 18 GPIO
- 8 pots via 4051
- 8 rotatifs via second 4051 partageant les lignes de selection
- 1 vrai switch direct

Pourquoi cette variante est la meilleure :
- elle tient dans le budget GPIO
- elle garde une petite marge
- elle reserve les shift registers a d'autres usages si besoin
- elle evite de surcharger le code avec trop de lecture digitale distribuee

### Si on veut re-utiliser le materiel V18

La meilleure re-utilisation probable est :
- garder l'idee des shift registers pour les controles digitaux
- mais seulement si les selecteurs rotatifs sont faciles a encoder en digital discret

Sinon :
- deux 4051 resteront plus simples a vivre

## Verdict net

Oui :
- une V2 expansion port avec 8 pots + 8 rotatifs + 1 switch est realiste sur RP2040

Mais seulement si :
- les controles sont multiplexes
- le bus reste discipline
- et qu'on n'essaie pas de cabler chaque controle en direct sur une pin dediee
