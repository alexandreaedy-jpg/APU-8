#include <Arduino.h>

/*
  NanoNesDataToggleD13
  --------------------
  Test electrique du fil NES D0 -> Nano D13.

  Le Nano force D13 LOW/HIGH toutes les 2 secondes.
  La LED integree suit le meme etat, car elle est aussi sur D13.
*/

static const uint8_t NES_DATA_PIN = 13;
static const unsigned long TOGGLE_MS = 2000;

bool dataHigh = false;
unsigned long lastToggleMs = 0;

void writeData() {
  digitalWrite(NES_DATA_PIN, dataHigh ? HIGH : LOW);
}

void setup() {
  pinMode(NES_DATA_PIN, OUTPUT);
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
