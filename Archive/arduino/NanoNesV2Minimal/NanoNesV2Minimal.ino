#include <Arduino.h>
#include <avr/interrupt.h>

static const uint8_t NES_OUT_PIN = 12;
static const uint8_t NES_D0_PIN = 2;
static const uint8_t NES_D3_PIN = 3;
static const uint8_t NES_D4_PIN = 4;

static const uint8_t CTRL_ATTACK_PIN = A0;
static const uint8_t CTRL_DECAY_PIN = A1;
static const uint8_t CTRL_SUSTAIN_PIN = A2;
static const uint8_t CTRL_RELEASE_PIN = A3;
static const uint8_t CONTROL_PINS[4] = {
  CTRL_ATTACK_PIN, CTRL_DECAY_PIN, CTRL_SUSTAIN_PIN, CTRL_RELEASE_PIN
};

static const uint8_t MIDI_CH_P1 = 12;
static const uint8_t MIDI_CH_P2 = 13;
static const uint8_t MIDI_CH_TRI = 14;

static const uint16_t START_HIGH_US = 1000;
static const uint16_t START_HIGH_TICKS = START_HIGH_US * 2;
static const bool V2_FIXED_ENV = true;

struct VoiceState {
  uint8_t note;
  uint8_t gate;
};

volatile uint8_t frameNotes[3] = {0, 0, 0};
volatile uint8_t latchedB0 = 0;
volatile uint8_t latchedB1 = 0;
volatile uint8_t latchedB2 = 0;
volatile uint8_t shiftMask = 1;
volatile bool transferActive = false;
volatile bool nesOutHigh = false;
volatile bool sawOutRise = false;
volatile uint16_t outRiseTicks = 0;

VoiceState voices[3] = {};
uint16_t filteredRaw[4] = {0, 0, 0, 0};
uint8_t lastEnvNibble[4] = {0, 4, 15, 2};
uint8_t midiRunningStatus = 0;
uint8_t midiData0 = 0;
uint8_t midiDataCount = 0;

inline void setParallelBits(bool d0High, bool d3High, bool d4High) {
  if (d0High) {
    PORTD &= (uint8_t)~_BV(PD2);
  } else {
    PORTD |= _BV(PD2);
  }

  if (d3High) {
    PORTD &= (uint8_t)~_BV(PD3);
  } else {
    PORTD |= _BV(PD3);
  }

  if (d4High) {
    PORTD &= (uint8_t)~_BV(PD4);
  } else {
    PORTD |= _BV(PD4);
  }
}

inline void outputCurrentBit() {
  if (!transferActive) {
    setParallelBits(false, false, false);
    return;
  }

  setParallelBits((latchedB0 & shiftMask) != 0,
                  (latchedB1 & shiftMask) != 0,
                  (latchedB2 & shiftMask) != 0);
}

inline void beginTransfer() {
  transferActive = true;
  shiftMask = 1;
  latchedB0 = frameNotes[0];
  latchedB1 = frameNotes[1];
  latchedB2 = frameNotes[2];

  outputCurrentBit();
}

inline void advanceTransfer() {
  shiftMask <<= 1;
  if (shiftMask == 0) {
    shiftMask = 1;
    transferActive = false;
    setParallelBits(false, false, false);
    return;
  }

  outputCurrentBit();
}

ISR(PCINT0_vect) {
  const bool outHigh = (PINB & _BV(PB4)) != 0;
  const uint16_t now = TCNT1;

  if (outHigh && !nesOutHigh) {
    outRiseTicks = now;
    sawOutRise = true;
  } else if (!outHigh && nesOutHigh) {
    if (sawOutRise) {
      const uint16_t highTicks = now - outRiseTicks;
      sawOutRise = false;
      if (highTicks >= START_HIGH_TICKS) {
        beginTransfer();
      } else if (transferActive) {
        advanceTransfer();
      }
    }
  }

  nesOutHigh = outHigh;
}

void setupTimingTimer() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = _BV(CS11);
  TCNT1 = 0;
  interrupts();
}

void setupNesPort() {
  pinMode(NES_D0_PIN, OUTPUT);
  pinMode(NES_D3_PIN, OUTPUT);
  pinMode(NES_D4_PIN, OUTPUT);
  pinMode(NES_OUT_PIN, INPUT_PULLUP);
  setParallelBits(false, false, false);

  PCICR |= _BV(PCIE0);
  PCMSK0 |= _BV(PCINT4);
}

int readAnalogStable(uint8_t pin) {
  analogRead(pin);
  delayMicroseconds(80);
  int a = analogRead(pin);
  int b = analogRead(pin);
  int c = analogRead(pin);
  return (a + b + c + 1) / 3;
}

uint8_t rawToNibble(int raw) {
  if (raw < 0) raw = 0;
  if (raw > 1023) raw = 1023;
  return (uint8_t)((raw * 15L + 511L) / 1023L);
}

uint8_t noteToFrameByte(uint8_t gate, uint8_t note) {
  if (!gate) {
    return 0;
  }
  if (note >= 127) {
    note = 126;
  }
  return (uint8_t)(note + 1);
}

void rebuildNoteFrame() {
  noInterrupts();
  frameNotes[0] = noteToFrameByte(voices[0].gate, voices[0].note);
  frameNotes[1] = noteToFrameByte(voices[1].gate, voices[1].note);
  frameNotes[2] = noteToFrameByte(voices[2].gate, voices[2].note);
  interrupts();
}

void rebuildEnvFrame() {
  (void)lastEnvNibble;
}

int voiceIndexFromChannel(uint8_t channel) {
  if (channel == MIDI_CH_P1) return 0;
  if (channel == MIDI_CH_P2) return 1;
  if (channel == MIDI_CH_TRI) return 2;
  return -1;
}

void noteOn(uint8_t channel, uint8_t note) {
  int voiceIndex = voiceIndexFromChannel(channel);
  if (voiceIndex < 0) {
    return;
  }

  voices[voiceIndex].note = note & 0x7F;
  voices[voiceIndex].gate = 1;
  rebuildNoteFrame();
}

void noteOff(uint8_t channel, uint8_t note) {
  int voiceIndex = voiceIndexFromChannel(channel);
  if (voiceIndex < 0) {
    return;
  }

  if (voices[voiceIndex].gate && voices[voiceIndex].note == (note & 0x7F)) {
    voices[voiceIndex].gate = 0;
    rebuildNoteFrame();
  }
}

void handleMidiMessage(uint8_t status, uint8_t data0, uint8_t data1) {
  uint8_t message = status & 0xF0;
  uint8_t channel = (status & 0x0F) + 1;

  if (message == 0x90 && data1 != 0) {
    noteOn(channel, data0 & 0x7F);
  } else if (message == 0x90 || message == 0x80) {
    noteOff(channel, data0 & 0x7F);
  }
}

void processMidi() {
  while (Serial.available() > 0) {
    uint8_t byteIn = (uint8_t)Serial.read();

    if (byteIn >= 0xF8) {
      continue;
    }

    if (byteIn & 0x80) {
      if (byteIn >= 0xF0) {
        midiRunningStatus = 0;
        midiDataCount = 0;
        continue;
      }
      midiRunningStatus = byteIn;
      midiDataCount = 0;
      continue;
    }

    if (midiRunningStatus == 0) {
      continue;
    }

    if (midiDataCount == 0) {
      midiData0 = byteIn;
      midiDataCount = 1;
    } else {
      handleMidiMessage(midiRunningStatus, midiData0, byteIn);
      midiDataCount = 0;
    }
  }
}

void processControls() {
  if (V2_FIXED_ENV) {
    return;
  }

  bool changed = false;
  for (uint8_t i = 0; i < 4; ++i) {
    int raw = readAnalogStable(CONTROL_PINS[i]);
    filteredRaw[i] = (uint16_t)((filteredRaw[i] * 3U + (uint16_t)raw + 2U) / 4U);
    uint8_t nibble = rawToNibble(filteredRaw[i]);
    if (nibble != lastEnvNibble[i]) {
      lastEnvNibble[i] = nibble;
      changed = true;
    }
  }

  if (changed) {
    rebuildEnvFrame();
  }
}

void setup() {
  Serial.begin(31250);
#if defined(DIDR0)
  DIDR0 |= _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D) | _BV(ADC3D);
#endif

  setupTimingTimer();
  setupNesPort();
  delay(300);

  if (!V2_FIXED_ENV) {
    for (uint8_t i = 0; i < 4; ++i) {
      filteredRaw[i] = (uint16_t)readAnalogStable(CONTROL_PINS[i]);
      lastEnvNibble[i] = rawToNibble(filteredRaw[i]);
    }
  }
  rebuildNoteFrame();
  rebuildEnvFrame();
}

void loop() {
  processMidi();
  processControls();
}
