#include <Arduino.h>

/*
  NanoNesDataToggle
  -----------------
  Test electrique ultra-simple du fil NES D0 -> Nano D2.

  Ce sketch ne lit pas OUT/D12 et n'utilise pas le MIDI.
  Il force D2 a alterner LOW/HIGH toutes les 2 secondes.

  Avec game_port_raw.nes lance sur la NES, la note doit changer
  toutes les 2 secondes si le fil noir D0 est bien vu par le port 2.
*/

static const uint8_t NES_DATA_PIN = 2;
static const uint8_t LED_PIN = 13;
static const unsigned long TOGGLE_MS = 2000;

bool dataHigh = false;
unsigned long lastToggleMs = 0;

void writeData() {
  digitalWrite(NES_DATA_PIN, dataHigh ? HIGH : LOW);
  digitalWrite(LED_PIN, dataHigh ? HIGH : LOW);
}

void setup() {
  pinMode(NES_DATA_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  dataHigh = false;
  lastToggleMs = millis();
  writeData();
}

void loop() {
  if ((unsigned long)(millis() - lastToggleMs) >= TOGGLE_MS) {
    lastToggleMs = millis();
    dataHigh = !dataHigh;
    writeData();
  }
}
