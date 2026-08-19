/*
  HardwareTest - Vérification du matériel NES MIDI
  -----------------------------------------------
  Teste les pins et la communication pour diagnostiquer les problèmes matériels.
*/

static const uint8_t NES_LATCH_PIN = 12;
static const uint8_t NES_DATA_PIN = 2;
static const uint8_t NES_D3_PIN = 3;
static const uint8_t NES_D4_PIN = 4;
static const uint8_t SELECTOR_PINS[4] = {5, 6, 7, 8};
static const uint8_t CONTROL_PINS[5] = {A0, A1, A2, A3, A4};

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("=== NES MIDI Hardware Test ===\n");
  
  // Configuration des pins
  pinMode(NES_LATCH_PIN, INPUT_PULLUP);
  pinMode(NES_DATA_PIN, OUTPUT);
  pinMode(NES_D3_PIN, OUTPUT);
  pinMode(NES_D4_PIN, OUTPUT);
  
  for (uint8_t i = 0; i < 4; ++i) {
    pinMode(SELECTOR_PINS[i], INPUT_PULLUP);
  }
  
  for (uint8_t i = 0; i < 5; ++i) {
    pinMode(CONTROL_PINS[i], INPUT);
  }
  
  // Test des pins de sortie
  Serial.println("--- Test pins de sortie (D2, D3, D4) ---");
  testOutputPin(NES_DATA_PIN, "D2 (DN0)");
  testOutputPin(NES_D3_PIN, "D3 (DN3)");
  testOutputPin(NES_D4_PIN, "D4 (DN4)");
  
  // Test des pins de sélecteur
  Serial.println("\n--- Test pins de sélecteur ---");
  for (uint8_t i = 0; i < 4; ++i) {
    testInputPin(SELECTOR_PINS[i], 
                 i == 0 ? "D5 (P1)" : 
                 i == 1 ? "D6 (P2)" : 
                 i == 2 ? "D7 (TRI)" : "D8 (GLOBAL)");
  }
  
  // Test des pins de contrôle analogiques
  Serial.println("\n--- Test pins de contrôle analogiques ---");
  for (uint8_t i = 0; i < 5; ++i) {
    testAnalogPin(CONTROL_PINS[i],
                  i == 0 ? "A0 (Attack)" :
                  i == 1 ? "A1 (Decay)" :
                  i == 2 ? "A2 (Sustain)" :
                  i == 3 ? "A3 (Release)" : "A4 (Duty)");
  }
  
  // Test LATCH
  Serial.println("\n--- Test pin LATCH ---");
  testInputPin(NES_LATCH_PIN, "D12 (LATCH)");
  
  // Test de communication parallèle
  Serial.println("\n--- Test communication parallèle ---");
  testParallelCommunication();
  
  // État des sélecteurs
  Serial.println("\n--- État des sélecteurs ---");
  for (uint8_t i = 0; i < 4; ++i) {
    bool state = digitalRead(SELECTOR_PINS[i]) == LOW;
    Serial.print(i == 0 ? "P1: " : i == 1 ? "P2: " : i == 2 ? "TRI: " : "GLOBAL: ");
    Serial.println(state ? "ON" : "OFF");
  }
  
  Serial.println("\n=== Test terminé ===");
}

void loop() {
  // Ne rien faire - le test s'exécute une seule fois dans setup()
}

void testOutputPin(uint8_t pin, const char* name) {
  Serial.print(name);
  Serial.print(" - ");
  
  digitalWrite(pin, LOW);
  delay(10);
  bool lowOk = (digitalRead(pin) == LOW);
  
  digitalWrite(pin, HIGH);
  delay(10);
  bool highOk = (digitalRead(pin) == HIGH);
  
  if (lowOk && highOk) {
    Serial.println("OK");
  } else {
    Serial.print("ERREUR (low=");
    Serial.print(lowOk);
    Serial.print(", high=");
    Serial.print(highOk);
    Serial.println(")");
  }
  
  digitalWrite(pin, LOW);
}

void testInputPin(uint8_t pin, const char* name) {
  Serial.print(name);
  Serial.print(" - ");
  
  pinMode(pin, INPUT_PULLUP);
  delay(10);
  bool pullup = digitalRead(pin) == HIGH;
  
  pinMode(pin, INPUT);
  delay(10);
  bool floating = digitalRead(pin) == LOW || digitalRead(pin) == HIGH;
  
  pinMode(pin, INPUT_PULLUP);
  
  if (pullup && floating) {
    Serial.println("OK");
  } else {
    Serial.print("ERREUR (pullup=");
    Serial.print(pullup);
    Serial.print(", floating=");
    Serial.print(floating);
    Serial.println(")");
  }
}

void testAnalogPin(uint8_t pin, const char* name) {
  Serial.print(name);
  Serial.print(" - ");
  
  int value = analogRead(pin);
  Serial.print(value);
  
  if (value >= 0 && value <= 1023) {
    Serial.println(" (OK)");
  } else {
    Serial.println(" (ERREUR)");
  }
}

void testParallelCommunication() {
  Serial.println("Test d'envoi de paquets sur D2, D3 et D4...");
  
  // Simuler un paquet sur D2
  digitalWrite(NES_DATA_PIN, LOW);
  delayMicroseconds(12);
  digitalWrite(NES_DATA_PIN, HIGH);
  delayMicroseconds(12);
  digitalWrite(NES_DATA_PIN, LOW);
  
  Serial.println("D2: Signal envoyé (LOW-HIGH-LOW)");
  
  // Simuler un paquet sur D3
  digitalWrite(NES_D3_PIN, LOW);
  delayMicroseconds(12);
  digitalWrite(NES_D3_PIN, HIGH);
  delayMicroseconds(12);
  digitalWrite(NES_D3_PIN, LOW);
  
  Serial.println("D3: Signal envoyé (LOW-HIGH-LOW)");
  
  // Simuler un paquet sur D4
  digitalWrite(NES_D4_PIN, LOW);
  delayMicroseconds(12);
  digitalWrite(NES_D4_PIN, HIGH);
  delayMicroseconds(12);
  digitalWrite(NES_D4_PIN, LOW);
  
  Serial.println("D4: Signal envoyé (LOW-HIGH-LOW)");
  
  digitalWrite(NES_DATA_PIN, LOW);
  digitalWrite(NES_D3_PIN, LOW);
  digitalWrite(NES_D4_PIN, LOW);
}
