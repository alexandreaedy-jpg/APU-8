#include <Arduino.h>
#include <avr/interrupt.h>
#include <string.h>

/*
  NanoNesPortTest
  ----------------
  Diagnostic minimal pour tester uniquement la liaison Nano -> port manette NES.

  Pas de MIDI.
  Pas de potards.
  Le Nano envoie un paquet APU-8 valide avec P2 active.

  Cablage NES attendu :
  - Vert  CLK -> D3 (ignore dans ce test)
  - Jaune OUT -> D12
  - Noir  D0  -> D13
  - Blanc GND -> GND
  - Rouge +5V non connecte pour ce test
*/

static const uint8_t NES_CLK_PIN = 3;
static const uint8_t NES_LATCH_PIN = 12;
static const uint8_t NES_DATA_PIN = 13;
static const uint8_t PACKET_SIZE = 58;
static const unsigned long START_HIGH_US = 2500;
static const unsigned long TRANSFER_TIMEOUT_US = 100000;

volatile uint8_t activePacket[PACKET_SIZE];
volatile uint8_t latchedPacket[PACKET_SIZE];
volatile uint8_t shiftByteIndex = PACKET_SIZE;
volatile uint8_t shiftBitMask = 1;
volatile bool nesLatchHigh = true;
volatile bool sawOutRise = false;
volatile bool transferActive = false;
volatile unsigned long outRiseUs = 0;
volatile unsigned long lastEdgeUs = 0;

uint8_t seq = 1;

inline void setNesDataBit(bool high) {
  DDRB |= _BV(PB5);
  if (high) {
    PORTB |= _BV(PB5);
  } else {
    PORTB &= (uint8_t)~_BV(PB5);
  }
}

inline void releaseNesDataBit() {
  PORTB &= (uint8_t)~_BV(PB5);
  DDRB &= (uint8_t)~_BV(PB5);
  transferActive = false;
}

inline void outputCurrentLatchedBit() {
  if (shiftByteIndex >= PACKET_SIZE) {
    setNesDataBit(0);
    return;
  }

  setNesDataBit((latchedPacket[shiftByteIndex] & shiftBitMask) != 0);
}

inline void beginControllerTransfer() {
  transferActive = true;
  for (uint8_t i = 0; i < PACKET_SIZE; ++i) {
    latchedPacket[i] = activePacket[i];
  }
  shiftByteIndex = 0;
  shiftBitMask = 1;
  outputCurrentLatchedBit();
}

inline void advanceControllerBit() {
  shiftBitMask <<= 1;
  if (shiftBitMask == 0) {
    shiftBitMask = 1;
    ++shiftByteIndex;
    if (shiftByteIndex >= PACKET_SIZE) {
      shiftByteIndex = 0;
    }
  }

  outputCurrentLatchedBit();
}

ISR(PCINT0_vect) {
  const bool latchHigh = (PINB & _BV(PB4)) != 0;
  const unsigned long now = micros();

  if (latchHigh && !nesLatchHigh) {
    outRiseUs = now;
    lastEdgeUs = now;
    sawOutRise = true;
  } else if (!latchHigh && nesLatchHigh) {
    if (sawOutRise) {
      const unsigned long highUs = now - outRiseUs;
      lastEdgeUs = now;
      sawOutRise = false;
      if (highUs >= START_HIGH_US) {
        beginControllerTransfer();
      } else if (transferActive) {
        advanceControllerBit();
      }
    }
  }

  nesLatchHigh = latchHigh;
}

void serviceTransferTimeout() {
  bool active;
  unsigned long lastEdge;

  noInterrupts();
  active = transferActive;
  lastEdge = lastEdgeUs;
  interrupts();

  if (active && (unsigned long)(micros() - lastEdge) >= TRANSFER_TIMEOUT_US) {
    noInterrupts();
    shiftByteIndex = PACKET_SIZE;
    releaseNesDataBit();
    interrupts();
  }
}

void buildTestPacket() {
  uint8_t packet[PACKET_SIZE];
  memset(packet, 0, sizeof(packet));

  packet[0] = 1;
  packet[8] = 0xA8;
  packet[9] = 0x58;

  // P2 note block.
  packet[13] = 64;
  packet[14] = 110;
  packet[15] = 1;

  // P2 controls.
  packet[26] = 2;    // duty
  packet[27] = 0;    // glide
  packet[28] = 0;    // LFO depth
  packet[29] = 0;    // LFO rate
  packet[37] = 0;    // attack
  packet[38] = 0;    // decay
  packet[39] = 127;  // sustain
  packet[40] = 0;    // release
  packet[53] = 1;  // P2 trigger

  noInterrupts();
  memcpy((void*)activePacket, packet, PACKET_SIZE);
  interrupts();
}

void setupNesPort() {
  pinMode(NES_CLK_PIN, INPUT_PULLUP);
  pinMode(NES_LATCH_PIN, INPUT_PULLUP);
  pinMode(NES_DATA_PIN, OUTPUT);
  setNesDataBit(0);

  nesLatchHigh = (PINB & _BV(PB4)) != 0;
  sawOutRise = false;
  outRiseUs = 0;
  lastEdgeUs = 0;

  noInterrupts();
  PCICR |= _BV(PCIE0);
  PCMSK0 |= _BV(PCINT4);
  PCIFR |= _BV(PCIF0);
  interrupts();
}

void setup() {
  setupNesPort();
  buildTestPacket();
  beginControllerTransfer();
}

void loop() {
}
