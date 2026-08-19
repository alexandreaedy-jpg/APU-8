#include <SoftwareSerial.h>

/*
  NanoMidiBridge
  ----------------
  Recoit du MIDI DIN IN a 31250 bauds et traduit les messages utiles
  en lignes texte compatibles avec bridge_separate_channels.py :

    NOTE,<CHANNEL>,<ON>,<NOTE>,<VEL>
    CC,<CHANNEL>,<CC>,<VALUE>
    PANIC

  Objectif V1 hardware :
  - MIDI DIN IN -> optocoupleur -> sortie logique vers D2
  - USB du Nano -> PC

  Le Nano ne gere ici que l'entree MIDI DIN.
  Les controles directs hardware viendront plus tard dans un autre sketch.
*/

static const uint8_t MIDI_RX_PIN = 2;
static const uint8_t MIDI_TX_PIN = 1;  // TX fantome, inutilise physiquement

static const unsigned long USB_BAUD = 115200;
static const unsigned long MIDI_BAUD = 31250;

SoftwareSerial midiSerial(MIDI_RX_PIN, MIDI_TX_PIN);

uint8_t runningStatus = 0;
uint8_t dataBytes[2];
uint8_t dataIndex = 0;
unsigned long lastRealtimeMs = 0;
unsigned long lastMidiDataMs = 0;
unsigned long lastDebugPrintMs = 0;
bool midiTrafficSeen = false;
uint16_t midiMessageCount = 0;

bool isChannelStatus(uint8_t status) {
  return status >= 0x80 && status <= 0xEF;
}

void sendNoteEvent(uint8_t channel, uint8_t on, uint8_t note, uint8_t vel) {
  Serial.print(F("NOTE,"));
  Serial.print(channel);
  Serial.print(F(","));
  Serial.print(on ? 1 : 0);
  Serial.print(F(","));
  Serial.print(note);
  Serial.print(F(","));
  Serial.println(vel);
  midiMessageCount++;
}

void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
  Serial.print(F("CC,"));
  Serial.print(channel);
  Serial.print(F(","));
  Serial.print(cc);
  Serial.print(F(","));
  Serial.println(value);
  midiMessageCount++;
}

void sendMappedCC(uint8_t channel, uint8_t cc, uint8_t value) {
  if (cc == 120 || cc == 123) {
    Serial.println(F("PANIC"));
    midiMessageCount++;
    return;
  }

  sendControlChange(channel, cc, value);
}

void handleChannelMessage(uint8_t status, uint8_t data0, uint8_t data1) {
  uint8_t message = status & 0xF0;
  uint8_t channel = (status & 0x0F) + 1;

  switch (message) {
    case 0x80:
      sendNoteEvent(channel, 0, data0 & 0x7F, data1 & 0x7F);
      break;

    case 0x90:
      if ((data1 & 0x7F) == 0) {
        sendNoteEvent(channel, 0, data0 & 0x7F, 0);
      } else {
        sendNoteEvent(channel, 1, data0 & 0x7F, data1 & 0x7F);
      }
      break;

    case 0xB0:
      sendMappedCC(channel, data0 & 0x7F, data1 & 0x7F);
      break;
  }
}

uint8_t expectedDataBytes(uint8_t status) {
  uint8_t message = status & 0xF0;

  switch (message) {
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

void processMidiByte(uint8_t b) {
  if (b >= 0xF8) {
    lastRealtimeMs = millis();
    return;
  }

  if (b & 0x80) {
    if (isChannelStatus(b)) {
      runningStatus = b;
    } else {
      runningStatus = 0;
    }
    dataIndex = 0;
    return;
  }

  if (runningStatus == 0) {
    return;
  }

  if (!midiTrafficSeen) {
    midiTrafficSeen = true;
    Serial.println(F("# DIN MIDI activity detected"));
  }

  lastMidiDataMs = millis();
  dataBytes[dataIndex++] = b & 0x7F;

  {
    const uint8_t needed = expectedDataBytes(runningStatus);
    if (needed == 0) {
      dataIndex = 0;
      return;
    }

    if (dataIndex >= needed) {
      if (needed == 2) {
        handleChannelMessage(runningStatus, dataBytes[0], dataBytes[1]);
      }
      dataIndex = 0;
    }
  }
}

void setup() {
  pinMode(MIDI_RX_PIN, INPUT_PULLUP);
  pinMode(MIDI_TX_PIN, OUTPUT);

  Serial.begin(USB_BAUD);
  midiSerial.begin(MIDI_BAUD);

  delay(200);
  Serial.println(F("# NanoMidiBridge ready"));
  Serial.println(F("# USB serial 115200"));
  Serial.println(F("# MIDI DIN IN on D2 at 31250"));
  Serial.println(F("# Protocol: NOTE,<CH>,<ON>,<NOTE>,<VEL> / CC,<CH>,<CC>,<VAL>"));
  Serial.println(F("# Waiting for DIN MIDI..."));
}

void loop() {
  while (midiSerial.available() > 0) {
    processMidiByte((uint8_t)midiSerial.read());
  }

  if (midiTrafficSeen && (millis() - lastDebugPrintMs > 3000UL)) {
    lastDebugPrintMs = millis();
    Serial.print(F("# MIDI messages sent: "));
    Serial.println(midiMessageCount);
  }

  if (midiTrafficSeen && lastMidiDataMs != 0 && (millis() - lastMidiDataMs > 5000UL) && (millis() - lastDebugPrintMs > 1000UL)) {
    lastDebugPrintMs = millis();
    Serial.println(F("# DIN MIDI quiet"));
  }

  // Heartbeat discret quand aucun message DIN n'est encore detecte.
  if (!midiTrafficSeen && (millis() - lastRealtimeMs > 2000UL)) {
    lastRealtimeMs = millis();
    Serial.println(F("# Idle, waiting for DIN MIDI"));
  }
}
