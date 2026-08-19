# HISTORIQUE DEBUG ADSR - NES MIDI V16-DN3

## Date : 28 Avril 2026 - Session de debug ADSR

---

## 🎯 OBJECTIF
Faire fonctionner les contrôles ADSR (Attack, Decay, Sustain, Release) via le canal D3 du port NES.

---

## ✅ CE QUI FONCTIONNE

### 1. Son de base (P1, P2, TRI)
- **Statut :** ✅ FONCTIONNE (revenu vers 1:00)
- **Cause du bug précédent :** `build.ps1` forçait `CONTROLLER_RX_P2_ONLY=1`
- **Correction :** Passé à `CONTROLLER_RX_P2_ONLY=0` dans `build.ps1`

### 2. Duty cycle via D3
- **Statut :** ✅ FONCTIONNE (confirmé par utilisateur)
- **Implication :** Le canal D3 arrive physiquement à la ROM et est lu correctement

### 3. Communication Arduino → NES
- D0 (notes) fonctionne
- D3 (duty) fonctionne
- Signaux validés à l'oscilloscope par l'utilisateur

---

## ❌ CE QUI NE FONCTIONNE PAS

### 1. ADSR (Attack/Decay/Sustain/Release)
- **Statut :** ❌ NON FONCTIONNEL
- **Symptôme :** Les potards ADSR n'ont aucun effet sur le son
- **Note :** L'utilisateur confirme que le sustain "faisait quelque chose" auparavant (pas le comportement souhaité, mais une réaction)

### 2. Affichage écran (diagnostic)
- **Statut :** ❌ ÉCRAN NOIR
- **Cause :** `ppu_boot_init()` perturbe l'initialisation du NES
- `crt0.s` désactive le PPU avant d'appeler `main()`, et la réactivation dans `ppu_boot_init()` est problématique

---

## 🔧 MODIFICATIONS APPORTÉES AU CODE

### Dans `main.c` (ROM) :

#### 1. Suppression lecture ADSR depuis D0 (lignes 662-683)
```c
// AVANT (problématique) :
BUS_P1_ATTACK = nibble_to_ctrl((controller_rx_compact[6] >> 4) & 0x0F);
// etc...

// APRÈS (commenté) :
// ADSR maintenant lu depuis D3 uniquement, pas depuis D0
// BUS_P1_ATTACK = nibble_to_ctrl(...);
```

**Raison :** D0 contenait des valeurs ADSR erronées qui écrasaient les bonnes valeurs de D3.

#### 2. Lecture ADSR depuis D3 (lignes 710-721)
```c
if (controller_rx_compact_d3[0] == 0xD3 && controller_rx_compact_d3[1] == 0x3D) {
    BUS_P1_ATTACK = nibble_to_ctrl((controller_rx_compact_d3[2] >> 4) & 0x0F);
    BUS_P2_ATTACK = nibble_to_ctrl(controller_rx_compact_d3[2] & 0x0F);
    BUS_P1_DECAY = nibble_to_ctrl((controller_rx_compact_d3[3] >> 4) & 0x0F);
    // ... etc
}
```
**Raison :** C'est ici que les vraies valeurs ADSR arrivent depuis l'Arduino.

#### 3. Tentative d'affichage debug (A1/A2) - NON FONCTIONNEL
Ajout de `ppu_write_u8_3()` dans `controller_rx_fast_p2_loop()` pour afficher les valeurs.
**Résultat :** Écran noir + pas de son.

### Dans `NanoNesV16DN3.ino` (Sketch Arduino) :

#### `rebuildPayload()` modifiée (ligne 375)
```c
void rebuildPayload() {
  noInterrupts();
  writeP2PacketFromState();
  writeD3PacketFromState();  // Reconstruire D3 immédiatement aussi
  markCurrentPacketDirty(NES_EVENT_REPEATS);
  interrupts();
}
```
**Raison :** Assurer que le paquet D3 est mis à jour immédiatement quand les contrôles changent.

#### `writeD3PacketFromState()` (lignes 431-439)
Construit le paquet D3 avec magic bytes `0xD3, 0x3D` et les valeurs ADSR en nibbles.

---

## 🔍 ANALYSE DES PROBLÈMES

### Problème 1 : Écran noir après ajout de `ppu_boot_init()`
**Hypothèse :** L'initialisation PPU dans `ppu_boot_init()` entre en conflit avec `crt0.s` qui fait déjà une init.
**Solution potentielle :** Ne pas appeler `ppu_boot_init()`, ou l'appeler différemment.

### Problème 2 : ADSR ne fonctionne pas malgré D3 reçu
**Éléments vérifiés :**
- ✅ D3 arrive physiquement (duty fonctionne)
- ✅ Sketch reconstruit D3 correctement
- ✅ ROM lit D3 et extrait les nibbles

**Pistes à explorer demain :**
1. **Timing de lecture D3 :** Peut-être que D0 et D3 sont lus dans le même cycle, créant une interférence
2. **Valeurs BUS_Px_ATTACK :** Vérifier avec un vrai debug (pas écran) que les valeurs changent
3. **Application des valeurs ADSR :** Vérifier que `ctrl_to_attack_rate()` etc. produisent des rates utilisables
4. **Enveloppe elle-même :** Peut-être que l'enveloppe fonctionne mais avec des valeurs trop faibles pour être perceptibles

---

## 📁 FICHIERS CONCERNÉS

| Fichier | Modifications |
|---------|--------------|
| `project-v16-DN3/main.c` | Lecture ADSR depuis D3 uniquement, tentatives debug écran |
| `project-v16-DN3/build.ps1` | `CONTROLLER_RX_P2_ONLY=0`, flags D3 corrects |
| `arduino/NanoNesV16DN3/NanoNesV16DN3.ino` | `writeD3PacketFromState()` dans `rebuildPayload()` |

---

## 🚀 PROCHAINES ÉTAPES SUGGÉRÉES

1. **Revenir à une ROM stable avec son** (sans l'écran debug qui casse tout)
2. **Ajouter un debug LED/audio** plutôt qu'écran (clignoter un registre si ADSR change)
3. **Vérifier le timing de lecture** D0/D3 dans `controller_rx_read_parallel_packets()`
4. **Tester avec des valeurs ADSR fixes** dans la ROM pour valider la chaîne d'enveloppe
5. **Comparer avec une ROM d'avril 2023** qui fonctionnait

---

## 📝 NOTES IMPORTANTES

- `build.ps1` écrase les `#define` de `main.c` - toujours modifier `build.ps1` pour les flags critiques
- `CONTROLLER_RX_FAST_P2=1` force la ROM à rester dans `controller_rx_fast_p2_loop()`
- L'Arduino envoie ADSR en nibbles (0-15), la ROM les reconvertit en 0-127 via `nibble_to_ctrl()`
- Le Duty fonctionne sur D3 = le canal physique D3 est OK

---

## État actuel de la ROM
La ROM actuelle (02:46) contient :
- ✅ Son de base fonctionnel
- ✅ Duty sur D3 fonctionnel
- ❌ Écran noir (ppu_boot_init retiré)
- ❌ ADSR non testable sans affichage

**Commit ID mental :** "Recherche ADSR D3 - écran debug cassé"
