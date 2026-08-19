# V2 Expansion Port - Liste de materiel E44 only

Date de reference : 2026-05-04

Objectif :
- preparer une version V2 d'APU-8 via l'expansion port NES
- sortir du protocole manette pour la voie critique
- pouvoir tout acheter chez E44 Nantes si la piste port manette finit par plafonner

Important :
- cette V2 suppose une reprise interne sur l'expansion port NES-001
- il n'existe pas de connecteur standard simple a acheter pour ce port
- la solution la plus realiste reste une nappe soudee en interne vers un connecteur standard

Changement cle par rapport a la BOM precedente :
- on remplace le maillon manquant 74LVC245A par une solution E44-only
- lecture NES -> RP2040 : 2 x 74HC4050
- ecriture RP2040 -> NES : 1 x 74HCT245
- et on passe les briques 5 V pilotees par le RP2040 en famille HCT quand c'est pertinent

## Noyau obligatoire

1. Carte microcontroleur
- 1 x RP2040 Pico WH
- role : microcontroleur principal de la V2
- prix de reference : 11.90 EUR TTC
- source : https://www.e44.com/kits-modules/raspberry/raspberry-pi-pico-wh-cortex-m0-bluetooth-5.2-wi-fi-2-4ghz-RASPBERRY-PI-PICO-WH.html

2. Translation NES -> RP2040
- 2 x 74HC4050
- role : abaisser proprement des signaux 5 V NES vers 3.3 V RP2040
- remarque : 1 boitier = 6 canaux, donc 2 boitiers donnent 12 voies, assez pour CPU D0..D7 plus quelques lignes de controle
- prix de reference : 0.45 EUR TTC piece
- total reference : 0.90 EUR TTC
- source : https://www.e44.com/composants/composants-actifs/circuits-integres/circuits-integres-logiques-ttl/serie-74hc/6xnon-inverting-buffer-dil16-74HC4050.html
- reference technique : https://www.nxp.com/docs/en/data-sheet/74HC4050.pdf

3. Translation RP2040 -> NES
- 1 x 74HCT245
- role : bus transceiver 8 bits 3-state pour envoyer du 3.3 V RP2040 vers logique 5 V NES
- prix de reference : 0.50 EUR TTC
- source : https://www.e44.com/composants/composants-actifs/circuits-integres/circuits-integres-logiques-ttl/serie-74hct/ic-digital-3-state-bus-transceiver-channels-8-dil20-74HCT245.html
- reference technique : https://assets.nexperia.com/documents/data-sheet/74AHC_AHCT245.pdf

4. Decodage simple
- 1 x 74HCT138
- role : decodage d'adresse ou de selection simple
- prix de reference : 0.40 EUR TTC
- source : https://www.e44.com/composants/composants-actifs/circuits-integres/circuits-integres-logiques-ttl/serie-74hct/ic-digital-line-decoder-demultiplexer-dip16-74HCT138.html

5. Registre latch
- 1 x 74HCT573
- role : latch simple pour interface de registres memory-mappes
- prix de reference : 0.75 EUR TTC
- source : https://www.e44.com/composants/composants-actifs/circuits-integres/circuits-integres-logiques-ttl/serie-74hct/ic-digital-3-state-latch-transparent-channels-8-dip20-74HCT573.html

6. Nettoyage / inversion / Schmitt
- 1 x 74HCT14
- role : remettre en forme certains fronts ou inverser proprement
- prix de reference : 0.70 EUR TTC
- source : https://www.e44.com/composants/composants-actifs/circuits-integres/circuits-integres-logiques-ttl/serie-74hct/ic-digital-schmitt-trigger-not-channels-6-inputs-1-dip14-74HCT14.html

## Passifs minimum

7. Decouplage logique
- 8 x condensateurs ceramique 100 nF
- role : un par CI logique + marge
- prix de reference : 0.10 EUR TTC piece
- total reference : 0.80 EUR TTC
- source : https://www.e44.com/composants/composants-passifs/condensateurs/condensateurs-ceramiques/condensateurs-ceramique-50-v-pas-5-08-mm/cond.-ceram.-5.08-mm-100nf-CC5100NF.html

8. Reservoir d'alimentation local
- 1 x condensateur radial entre 47 uF et 100 uF
- role : stabilisation locale d'alimentation
- conseil : prendre n'importe quelle reference dispo au comptoir en 16 V ou plus

9. Resistances
- minimum utile : 1k, 4.7k, 10k
- usage : serie de protection, pull-up, pull-down, lignes de controle
- conseil pratique : prendre 5 a 10 pieces de chaque valeur
- exemples E44 :
  - 1k : https://www.e44.com/composants/composants-passifs/resistances/resistances-precision/0-25-w/resistance-couche-metallique-1-4w-1-kohms-1-MA1K0.html
  - 4.7k : https://www.e44.com/composants/composants-passifs/resistances/resistances-precision/0-25-w/resistance-couche-metallique-1-4w-4.7-kohms-1-MA4K7.html
  - 10k : https://www.e44.com/composants/composants-passifs/resistances/resistances-precision/0-25-w/resistance-couche-metallique-1-4w-10-kohms-1-MA10K0.html

## Connectique pour la V2

10. Nappe standard
- 1 m de cable en nappe 20 conducteurs multicolore
- role : liaison interne propre vers breakout / module
- prix de reference : 2.00 EUR TTC
- source : https://www.e44.com/cablages/cables/cables-en-nappe/cable-en-nappe-20-conducteurs-0.08mm2-pas-1.27mm-1m-mulicolore-FNG20-C.html

11. Connecteur IDC femelle pour la nappe
- 1 x HE10F20
- role : sertir la nappe 20 conducteurs
- prix de reference : 0.50 EUR TTC
- source : https://www.e44.com/connectique/connecteurs/fiches/IDC-20b/Femelle/he10-femelle-sertir-20-pins-HE10F20.html

12. Embase male cote module
- 1 x HE10MD20
- role : interface detachable cote breakout / carte V2
- prix de reference : 0.80 EUR TTC
- source : https://www.e44.com/connectique/connecteurs/embases-ci/IDC-20b/Male/he10-male-droit-20-pins-HE10MD20.html

## Proto et confort

13. Breadboard
- 1 x SPB830+
- role : premier proto de validation
- prix de reference : 9.90 EUR TTC
- source : https://www.e44.com/outillage/realisation-circuit-imprime/plaques-d-experimentations/plaque-montage-rapide-830-trous-140-fils-raccordement-SPB830-.html

14. Plaque pour figer une revision
- 1 x VEROP+ ou VEROB+
- role : figer une version propre apres validation breadboard
- budget de reference : 7.50 EUR TTC
- pastilles : https://www.e44.com/outillage/realisation-circuit-imprime/plaques-veroboard/plaque-veroboard-epoxy-pastilles-cuivrees-au-pas-2.54mm-dim-100-160mm-VEROP-.html
- bandes : https://www.e44.com/outillage/realisation-circuit-imprime/plaques-veroboard/plaque-veroboard-epoxy-bandes-cuivrees-au-pas-2.54mm-dim-100-160mm-VEROB-.html

## Budget par niveau

### Niveau 1 - minimum credible

Comprend :
- RP2040 Pico WH
- 2 x 74HC4050
- 1 x 74HCT245
- 1 x 74HCT138
- 1 x 74HCT573
- 1 x 74HCT14
- 8 x 100 nF
- 1 x electrolitique 47 a 100 uF
- 1 m nappe 20 conducteurs
- 1 x HE10F20
- 1 x HE10MD20
- resistances ciblees 1k / 4.7k / 10k

Budget estime :
- environ 18 a 22 EUR TTC selon les resistances prises

### Niveau 2 - proto confortable

Niveau 1 plus :
- 1 x breadboard SPB830+

Budget estime :
- environ 28 a 32 EUR TTC

### Niveau 3 - proto propre / evolutif

Niveau 2 plus :
- 1 x plaque VEROP+ ou VEROB+

Budget estime :
- environ 35 a 40 EUR TTC

## Ce que je ne prendrais pas comme solution principale

15. VMA410
- reference : https://www.e44.com/kits-modules/arduino/module-convertisseur-3.3-5-ttl-logic-level-VMA410.html
- verdict : utile pour du bench ou une petite ligne lente, mais pas comme coeur du bus V2
- raisons :
  - seulement 2 voies utiles par sens
  - basee sur diviseur + MOSFET
  - pas de vrai bus transceiver 3-state
  - moins adaptee a un bus CPU NES que la combinaison 74HC4050 + 74HCT245

## Lignes d'expansion les plus interessantes pour V2

- CPU D0..D7
- A15
- OUT0
- R/W si besoin ensuite
- /IRQ plus tard, avec prudence et resistance serie
- +5V
- GND

## Verdict

Oui, une V2 expansion port en version E44-only est realiste.

Le bon compromis aujourd'hui est :
- 2 x 74HC4050 pour lire le NES vers le RP2040
- 1 x 74HCT245 pour ecrire du RP2040 vers le NES
- HCT138 / HCT573 / HCT14 pour les briques 5 V pilotees par le RP2040

Le vrai cout cache restera surtout :
- le temps mecanique
- la reprise interne sur la console
- les essais de cablage

Mais le cout pur composants reste raisonnable.
