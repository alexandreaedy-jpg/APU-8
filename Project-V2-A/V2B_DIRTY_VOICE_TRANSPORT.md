# V2B Dirty Voice Transport

Date de reference : 2026-05-18

## 1) Pourquoi V2B

Les tests `Log_25-Midi_test18` a `Log_25-Midi_test20` ont clarifie le probleme :

- quand `PULSE_FRAME` etait remplaçable, certaines notes courtes etaient perdues
- quand on a preserve les edges, la queue a fini par saturer
- quand on a compacte par voix, le comportement s'est rapproche de `V18`, mais le transport actuel reste trop fragile en section dense

Conclusion :

- le probleme principal n'est plus seulement le timing bus
- le probleme est aussi la forme du transport
- il faut sortir du modele "FIFO globale de snapshots"

Le bon cap est un transport :

- centre sur l'etat musical des voix
- avec des edges de note prioritaires
- avec des etats continus coalescables
- et une consommation pilotee par la NES

## 2) Ce que montrent NESizer et ChipMaestro

### NESizer

Refs :

- [note_stack.c](C:/Users/mto1/Documents/NES_DEV/_ref_NESizer2/src/note_stack/note_stack.c)
- [assigner.c](C:/Users/mto1/Documents/NES_DEV/_ref_NESizer2/src/assigner/assigner.c)
- [apu.c](C:/Users/mto1/Documents/NES_DEV/_ref_NESizer2/src/apu/apu.c)
- [2a03.c](C:/Users/mto1/Documents/NES_DEV/_ref_NESizer2/src/io/2a03.c)

Lecon principale :

- la pile de notes gere l'intention musicale
- l'assigner decide quelle voix joue quoi
- l'APU est ensuite rafraichi par deltas

Points clefs :

- `note_stack_pop()` rejoue explicitement la note precedente quand il reste une note tenue
- `assigner_notify_note_on/off()` travaille par groupe/canal, pas par octet transporte
- `apu_update_handler()` met a jour un canal a la fois
- `io_write_changed()` n'ecrit que les registres qui changent reellement

NESizer ne demande pas au transport de "reconstruire" la musique.
La musique existe deja en etat local.

### ChipMaestro

Refs :

- [ChipMaestro.ino](C:/Users/mto1/Documents/NES_DEV/_ref_ChipMaestro_thquinn/ChipMaestro/ChipMaestro.ino)
- [Channel.cpp](C:/Users/mto1/Documents/NES_DEV/_ref_ChipMaestro_thquinn/Channel.cpp)
- [Channel.h](C:/Users/mto1/Documents/NES_DEV/_ref_ChipMaestro_thquinn/Channel.h)

Lecon principale :

- chaque canal construit un petit paquet coherent
- seuls les canaux `changed` sont emis
- des ecritures inutiles sont retirees

Points clefs :

- `Channel::setPacket()` produit un paquet court par canal
- les pulses evitent deja certaines re-ecritures inutiles (`dirtyPulse`, `lastHigh`)
- la boucle principale n'envoie que les canaux marques `changed`
- `toNES()` attend explicitement la destination avant d'avancer

ChipMaestro envoie des paquets de canal, pas une pluie de micro-ordres ambigus.

## 3) Diagnostic sur notre V2A actuel

Sur `step25 midi in p1/p2`, on a teste plusieurs variantes :

- `PULSE_FRAME` 16-bit : coherent musicalement sur le papier, mais trop lourd en charge dense
- remplacement agressif : faible latence, mais pertes de notes courtes
- preservation des edges : plus fidele, mais saturation de queue
- compact par voix : meilleure direction, mais la FIFO globale reste encore la mauvaise abstraction

Le probleme de fond est le suivant :

- les edges de note ne sont pas remplaçables
- les controles continus, eux, doivent etre remplaçables
- un seul type de file ne convient pas bien aux deux

## 4) Architecture V2B recommandee

### 4.1) Separation forte

V2B doit separer 3 couches :

1. `musical state`
2. `transport state`
3. `apu apply`

### 4.2) Musical state cote Pico

Le Pico doit conserver un etat local complet :

- `note stack` par source MIDI utile
- etat de voix `P1 / P2 / TRI / NOI / DMC`
- gate
- note courante
- trigger counter
- controles courants utiles a la voix

Le MIDI met a jour cet etat local d'abord.

Le transport n'est qu'une projection compacte de cet etat.

### 4.3) Transport state cote Pico

V2B ne doit plus avoir une seule FIFO uniforme.

Il faut au minimum :

- une petite `edge queue` par voix pour les `Note On / Note Off / Resume`
- un `dirty state slot` par voix pour l'etat stable le plus recent
- un `dirty control slot` global pour les CC / duty / LFO / ADSR

Regle :

- les `edge queue` ne remplacent pas un edge non lu
- les `state slots` remplacent librement l'etat precedent tant qu'il n'est pas consomme
- les `control slots` remplacent librement l'etat precedent

### 4.4) Priorite de service cote ROM

La ROM doit servir dans cet ordre :

1. `voice edge`
2. `voice state`
3. `global control`

Ainsi :

- les attaques et releases restent prioritaires
- les etats soutenus et CC ne bloquent pas les notes

## 5) Format V2B minimal conseille

Le plus rentable pour repartir proprement est :

- garder le bus `OUT0..OUT2 + D0..D4`
- garder le principe `READ_STATUS / READ_REG / READ_VALUE / ACK`
- changer la semantique des paquets, pas tout le cablage

### 5.1) Types de paquets

Premier set conseille :

- `EDGE_P1`
- `EDGE_P2`
- `EDGE_TRI`
- `STATE_P1`
- `STATE_P2`
- `STATE_TRI`
- `STATE_NOI`
- `STATE_DMC`
- `CTRL_GLOBAL`

### 5.2) Payload minimal de phase 1

Pour relancer vite et fiablement :

- `EDGE_P1` = `note(7) + gate(1)`
- `EDGE_P2` = `note(7) + gate(1)`
- `EDGE_TRI` = `note(7) + gate(1)`

Interpretation ROM :

- `gate=1` = appliquer note + retrigger
- `gate=0` = release / stop

Donc un `resume` redevient simplement un nouveau `EDGE_* gate=1`.

### 5.3) State packets

Quand la phase note-edge sera stable, ajouter :

- `STATE_P1` : duty, glide, vibrato depth/rate, volume mode
- `STATE_P2` : idem
- `STATE_TRI` : glide / mute / mode
- `STATE_NOI` : period, mode, volume
- `STATE_DMC` : sample id / trigger / mode
- `CTRL_GLOBAL` : tempo-sync, ADSR partages si encore necessaires

Chaque `STATE_*` doit etre :

- compact
- bitmaske
- remplaçable tant qu'il n'est pas lu

## 6) Contrat ROM recommande

La ROM V2B doit :

- appliquer un `EDGE_*` immediatement
- ne pas attendre la fin d'un gros batch pour rendre l'attaque audible
- continuer a amortir les `STATE_*` et `CTRL_*`

Autrement dit :

- `edge` = hot path
- `state/control` = cool path

## 7) Pourquoi cette direction est plus extensible

Cette architecture resout mieux la peur legitime suivante :

> "si P2 ajoute deja autant de latence, que va-t-il se passer avec TRI + Noise + Sample ?"

Avec V2B :

- `TRI` ajoute sa propre petite edge queue
- `NOI` et `DMC` peuvent etre traites comme voix speciales
- les controles continus ne parasitent plus directement les attaques

On n'est plus oblige d'envoyer :

- soit un snapshot global trop lourd
- soit une pluie de micro-evenements sans priorite

## 8) Plan d'execution conseille

### Phase 0

Geler les constats V2A :

- `PULSE_FRAME` trop lourd en charge dense
- une FIFO globale unique ne convient pas

### Phase 1

Implementer `V2B pulse-only` :

- `EDGE_P1`
- `EDGE_P2`
- `dirty state/control` inactifs ou minimaux

Objectif :

- retrouver au moins la tenue musicale de `V18` sur `P1 + P2`

Etat au 2026-05-18 :

- premiere implementation en cours dans `v2a_midi_in_p1p2`
- opcodes specialises `STATUS / P1_LO / P1_HI / P2_LO / P2_HI / ACK_P1 / ACK_P2`
- files/pending separes pour `P1` et `P2`

### Phase 2

Ajouter `EDGE_TRI`.

### Phase 3

Ajouter `STATE_*` remplaçables.

### Phase 4

Ajouter `NOI / DMC`.

## 9) Regles a garder

- ne jamais demander au transport de reconstituer seul la musique
- ne jamais melanger `edge critique` et `state remplaçable` dans la meme politique FIFO
- toujours favoriser les paquets par voix ou par canal
- n'ecrire que ce qui a vraiment change

## 10) Verdict

Les refs `NESizer` et `ChipMaestro` ne poussent pas vers un "mega frame global".

Elles poussent vers :

- etat local fort
- assignation musicale claire
- paquets courts par voix/canal
- ecritures deltas

La prochaine vraie etape du projet ne doit donc pas etre un nouveau tweak de `PULSE_FRAME`.

La prochaine vraie etape doit etre :

- **V2B = dirty voice transport avec edge queues prioritaires et state slots remplaçables**
