#include <Arduino.h>

/*
  NanoMidiRxTest
  ---------------
  Test minimal pour valider le MIDI DIN sur RX/D0.

  Cablage attendu :
  - sortie opto MIDI -> RX/D0
  - GND commun

  Comportement :
  - clignotement court : octet brut recu
  - clignotement plus long : vrai message MIDI canal reconnu
  - aucun Serial Monitor utile ici, RX est utilise par le MIDI DIN
*/

static const unsigned long MIDI_BAUD = 31250;
static const uint8_t MIDI_RX_PIN = 0;
static const uint8_t LED_PIN = LED_BUILTIN;
static const unsigned long LED_HOLD_MS = 40;
static const unsigned long LED_VALID_HOLD_MS = 140;

unsigned long ledOffAt = 0;
uint32_t byteCount = 0;
uint8_t runningStatus = 0;
uint8_t dataIndex = 0;
uint8_t neededData = 0;

uint8_t expectedDataBytes(uint8_t status) {
  switch (status & 0xF0) {
    case 0x80:
    case 0x90:
    case 0xA0:
    case 0xB0:
    case 0xE0:
      return 2;
    case 0xC0:
    case 0xD0:
      return 1;
  }
  return 0;
}

void pulseLed(unsigned long holdMs) {
  digitalWrite(LED_PIN, HIGH);
  ledOffAt = millis() + holdMs;
}

void processMidiByte(uint8_t b) {
  if (b >= 0xF8) {
    return;
  }

  if (b & 0x80) {
    if (b >= 0x80 && b <= 0xEF) {
      runningStatus = b;
      neededData = expectedDataBytes(runningStatus);
      dataIndex = 0;
    } else {
      runningStatus = 0;
      neededData = 0;
      dataIndex = 0;
    }
    return;
  }

  if (!runningStatus || neededData == 0) {
    return;
  }

  ++dataIndex;
  if (dataIndex >= neededData) {
    dataIndex = 0;
    pulseLed(LED_VALID_HOLD_MS);
  }
}

void setup() {
  pinMode(MIDI_RX_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(MIDI_BAUD);
}

void loop() {
  while (Serial.available() > 0) {
    uint8_t b = (uint8_t)Serial.read();
    ++byteCount;
    pulseLed(LED_HOLD_MS);
    processMidiByte(b);
  }

  if (ledOffAt != 0 && (long)(millis() - ledOffAt) >= 0) {
    digitalWrite(LED_PIN, LOW);
    ledOffAt = 0;
  }
}
