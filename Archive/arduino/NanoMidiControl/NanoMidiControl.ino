#include <Arduino.h>
#include <SoftwareSerial.h>

/*
  NanoMidiControl
  ---------------
  Sketch fusionne :
  - MIDI DIN IN -> D2 via optocoupleur
  - controles hardware -> selecteurs + potards
  - USB serial -> bridge_separate_channels.py

  Protocoles envoyes :
    NOTE,<CHANNEL>,<ON>,<NOTE>,<VEL>
    CC,<CHANNEL>,<CC>,<VALUE>
    PANIC

  Cablage :

  MIDI DIN :
  - optocoupleur out -> D2
  - D1 reserve comme TX fantome SoftwareSerial (non utilise physiquement)

  Selecteur 4 positions (contacts separes vers GND, INPUT_PULLUP) :
  - D4  -> P1
  - D5  -> P2
  - D6  -> TRI
  - D8  -> GLOBAL

  Switch ARP sync (contact vers GND, INPUT_PULLUP) :
  - D7  -> Sync on/off

  Selecteur destination LFO 3 positions (contacts separes vers GND, INPUT_PULLUP) :
  - D9  -> Pitch
  - D10 -> Duty
  - D11 -> Amp

  Analogiques :
  - A0  -> Attack
  - A1  -> Decay
  - A2  -> Sustain
  - A3  -> Release
  - A4  -> Ctrl 1
  - A5  -> Ctrl 2
  - A6  -> Ctrl 3
  - A7  -> Ctrl 4
*/

static const uint8_t MIDI_RX_PIN = 2;
static const uint8_t MIDI_TX_PIN = 1;

static const unsigned long USB_BAUD = 115200;
static const unsigned long MIDI_BAUD = 31250;
static const unsigned long STARTUP_DELAY_MS = 250;
static const unsigned long CONTROL_SCAN_MS = 2;
static const bool DEBUG_CONTROL_EVENTS = false;
static const uint8_t ANALOG_READ_SAMPLES = 3;
static const uint8_t SELECTOR_STABLE_SCANS = 4;

SoftwareSerial midiSerial(MIDI_RX_PIN, MIDI_TX_PIN);

enum TargetChannel : uint8_t {
  TARGET_P1 = 12,
  TARGET_P2 = 13,
  TARGET_TRI = 14,
  TARGET_NOISE = 15,
  TARGET_GLOBAL = 16
};

enum SelectorIndex : uint8_t {
  SEL_P1 = 0,
  SEL_P2 = 1,
  SEL_TRI = 2,
  SEL_GLOBAL = 3,
  SEL_COUNT = 4
};

static const uint8_t SELECTOR_PINS[SEL_COUNT] = {4, 5, 6, 8};
static const TargetChannel DEFAULT_TARGET = TARGET_P2;
static const uint8_t ARP_SYNC_PIN = 7;

enum LfoTargetMode : uint8_t {
  LFO_TO_PITCH = 0,
  LFO_TO_DUTY = 1,
  LFO_TO_AMP = 2
};

static const uint8_t LFO_TARGET_PINS[3] = {9, 10, 11};

enum ControlIndex : uint8_t {
  CTRL_ATTACK = 0,
  CTRL_DECAY = 1,
  CTRL_SUSTAIN = 2,
  CTRL_RELEASE = 3,
  CTRL_1 = 4,
  CTRL_2 = 5,
  CTRL_3 = 6,
  CTRL_4 = 7,
  CTRL_COUNT = 8
};

static const uint8_t CONTROL_PINS[CTRL_COUNT] = {A0, A1, A2, A3, A4, A5, A6, A7};
static const bool CONTROL_ENABLED[CTRL_COUNT] = {
  true, true, true, true, true, true, true, true
};

int lastAnalogRaw[CTRL_COUNT];
int filteredAnalogRaw[CTRL_COUNT];
uint8_t lastSentValue[CTRL_COUNT];
uint8_t lastSentMappedValue[CTRL_COUNT];
uint8_t pendingMappedValue[CTRL_COUNT];
uint8_t pendingMappedCount[CTRL_COUNT];

TargetChannel currentTarget = DEFAULT_TARGET;
TargetChannel lastTarget = DEFAULT_TARGET;
LfoTargetMode currentLfoTarget = LFO_TO_PITCH;
LfoTargetMode lastLfoTarget = LFO_TO_PITCH;
bool currentArpSyncEnabled = false;
TargetChannel pendingTarget = DEFAULT_TARGET;
LfoTargetMode pendingLfoTarget = LFO_TO_PITCH;
bool pendingArpSyncEnabled = false;
uint8_t pendingTargetCount = 0;
uint8_t pendingLfoTargetCount = 0;
uint8_t pendingArpSyncCount = 0;

unsigned long lastControlScanMs = 0;
uint8_t nextControlIndex = 0;

uint8_t runningStatus = 0;
uint8_t dataBytes[2];
uint8_t dataIndex = 0;
unsigned long lastRealtimeMs = 0;
unsigned long lastMidiDataMs = 0;
unsigned long lastDebugPrintMs = 0;
bool midiTrafficSeen = false;
uint16_t midiMessageCount = 0;

uint8_t mapRawToMidi(int raw) {
  if (raw < 0) raw = 0;
  if (raw > 1023) raw = 1023;
  return (uint8_t)((raw * 127L) / 1023L);
}

uint8_t quantizeOnOff(uint8_t value) {
  return value >= 64 ? 127 : 0;
}

uint8_t quantizeDuty4(uint8_t value) {
  if (value < 32) return 0;
  if (value < 64) return 43;
  if (value < 96) return 86;
  return 127;
}

uint8_t quantizeVibratoRate(uint8_t value) {
  if (value < 8) return 0;
  if (value < 18) return 14;
  if (value < 30) return 24;
  if (value < 44) return 36;
  if (value < 58) return 50;
  if (value < 72) return 66;
  if (value < 86) return 84;
  if (value < 98) return 100;
  if (value < 108) return 112;
  if (value < 118) return 120;
  return 127;
}

uint8_t quantizeArpSpeed(uint8_t value, bool syncEnabled) {
  if (value < 8) return 0;

  if (syncEnabled) {
    if (value < 18) return 16;
    if (value < 34) return 32;
    if (value < 50) return 48;
    if (value < 66) return 64;
    if (value < 82) return 80;
    if (value < 98) return 96;
    return 112;
  }

  if (value < 16) return 8;
  if (value < 28) return 20;
  if (value < 40) return 32;
  if (value < 54) return 44;
  if (value < 68) return 56;
  if (value < 82) return 72;
  if (value < 96) return 88;
  if (value < 110) return 104;
  return 116;
}

uint8_t stretchMidiRange(uint8_t value, uint8_t lowDeadzone, uint8_t highClamp) {
  if (value <= lowDeadzone) {
    return 0;
  }
  if (value >= highClamp) {
    return 127;
  }

  return (uint8_t)(((unsigned int)(value - lowDeadzone) * 127U + ((highClamp - lowDeadzone) / 2U)) / (unsigned int)(highClamp - lowDeadzone));
}

uint8_t zeroClampForControl(ControlIndex controlIndex, uint8_t value) {
  uint8_t threshold = 0;

  switch (controlIndex) {
    case CTRL_ATTACK:
      threshold = 14;
      break;
    case CTRL_DECAY:
      threshold = 8;
      break;
    case CTRL_SUSTAIN:
      threshold = 8;
      break;
    case CTRL_RELEASE:
      threshold = 16;
      break;
    case CTRL_1:
      threshold = 10;
      break;
    case CTRL_2:
      threshold = 4;
      break;
    case CTRL_3:
      threshold = 10;
      break;
    case CTRL_4:
      threshold = 10;
      break;
    default:
      threshold = 0;
      break;
  }

  if (value <= threshold) {
    return 0;
  }
  return value;
}

uint8_t normalizeMidiForControl(ControlIndex controlIndex, uint8_t value) {
  value = zeroClampForControl(controlIndex, value);
  if (controlIndex == CTRL_ATTACK) {
    return stretchMidiRange(value, 18, 122);
  }
  if (controlIndex == CTRL_DECAY) {
    return stretchMidiRange(value, 8, 120);
  }
  if (controlIndex == CTRL_SUSTAIN) {
    return stretchMidiRange(value, 8, 123);
  }
  if (controlIndex == CTRL_RELEASE) {
    return stretchMidiRange(value, 14, 120);
  }
  if (controlIndex == CTRL_1) {
    return stretchMidiRange(value, 8, 123);
  }
  if (controlIndex == CTRL_2) {
    return stretchMidiRange(value, 4, 123);
  }
  if (controlIndex == CTRL_3) {
    return stretchMidiRange(value, 18, 123);
  }
  if (controlIndex == CTRL_4) {
    return quantizeVibratoRate(stretchMidiRange(value, 6, 123));
  }
  return value;
}

uint8_t analogThresholdForControl(ControlIndex controlIndex) {
  if (controlIndex >= CTRL_1) {
    return 6;
  }
  return 4;
}

int filteredRawForControl(ControlIndex controlIndex, int previousFiltered, int raw) {
  if (controlIndex >= CTRL_1) {
    int filtered = (previousFiltered * 3 + raw + 2) / 4;
    if (abs(filtered - raw) <= 2) {
      return raw;
    }
    return filtered;
  }
  {
    int filtered = (previousFiltered + raw + 1) / 2;
    if (abs(filtered - raw) <= 1) {
      return raw;
    }
    return filtered;
  }
}

uint8_t stableScansRequiredForControl(ControlIndex controlIndex) {
  if (controlIndex <= CTRL_RELEASE) {
    return 1;
  }
  return 2;
}

int readStableAnalog(uint8_t pin) {
  long acc = 0;

  analogRead(pin);
  delayMicroseconds(80);

  for (uint8_t i = 0; i < ANALOG_READ_SAMPLES; ++i) {
    acc += analogRead(pin);
    delayMicroseconds(20);
  }

  return (int)((acc + (ANALOG_READ_SAMPLES / 2)) / ANALOG_READ_SAMPLES);
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

void sendCC(uint8_t channel, uint8_t cc, uint8_t value) {
  Serial.print(F("CC,"));
  Serial.print(channel);
  Serial.print(F(","));
  Serial.print(cc);
  Serial.print(F(","));
  Serial.println(value);
  midiMessageCount++;
}

void sendRealtimeEvent(const __FlashStringHelper* name) {
  Serial.println(name);
  midiMessageCount++;
}

void sendPanic() {
  Serial.println(F("PANIC"));
  midiMessageCount++;
}

void sendMappedCC(uint8_t channel, uint8_t cc, uint8_t value) {
  if (cc == 120 || cc == 123) {
    sendPanic();
    return;
  }
  sendCC(channel, cc, value);
}

void logControlEvent(ControlIndex controlIndex, TargetChannel target, uint8_t cc, uint8_t value) {
  if (!DEBUG_CONTROL_EVENTS) {
    return;
  }
  Serial.print(F("# CTRL A"));
  Serial.print((int)controlIndex);
  Serial.print(F(" -> CH"));
  Serial.print((uint8_t)target);
  Serial.print(F(" CC"));
  Serial.print(cc);
  Serial.print(F(" = "));
  Serial.println(value);
}

bool isChannelStatus(uint8_t status) {
  return status >= 0x80 && status <= 0xEF;
}

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

void processMidiByte(uint8_t b) {
  if (b == 0xF8) {
    sendRealtimeEvent(F("CLOCK"));
    lastRealtimeMs = millis();
    return;
  }
  if (b == 0xFA) {
    sendRealtimeEvent(F("START"));
    lastRealtimeMs = millis();
    return;
  }
  if (b == 0xFB) {
    sendRealtimeEvent(F("CONTINUE"));
    lastRealtimeMs = millis();
    return;
  }
  if (b == 0xFC) {
    sendRealtimeEvent(F("STOP"));
    lastRealtimeMs = millis();
    return;
  }
  if (b >= 0xF8) {
    lastRealtimeMs = millis();
    return;
  }

  if (b & 0x80) {
    runningStatus = isChannelStatus(b) ? b : 0;
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

bool readTargetSelector(TargetChannel* outTarget) {
  for (uint8_t i = 0; i < SEL_COUNT; ++i) {
    if (digitalRead(SELECTOR_PINS[i]) == LOW) {
      switch (i) {
        case SEL_P1:     *outTarget = TARGET_P1; return true;
        case SEL_P2:     *outTarget = TARGET_P2; return true;
        case SEL_TRI:    *outTarget = TARGET_TRI; return true;
        case SEL_GLOBAL: *outTarget = TARGET_GLOBAL; return true;
      }
    }
  }
  return false;
}

bool readArpSyncSwitch() {
  return digitalRead(ARP_SYNC_PIN) == LOW;
}

bool readLfoTargetSelector(LfoTargetMode* outMode) {
  if (digitalRead(LFO_TARGET_PINS[0]) == LOW) { *outMode = LFO_TO_PITCH; return true; }
  if (digitalRead(LFO_TARGET_PINS[1]) == LOW) { *outMode = LFO_TO_DUTY;  return true; }
  if (digitalRead(LFO_TARGET_PINS[2]) == LOW) { *outMode = LFO_TO_AMP;   return true; }
  return false;
}

uint8_t lfoTargetModeToCcValue(LfoTargetMode mode) {
  switch (mode) {
    case LFO_TO_DUTY: return 64;
    case LFO_TO_AMP: return 127;
    case LFO_TO_PITCH:
    default:
      return 0;
  }
}

void sendLfoTargetForTarget(TargetChannel target, LfoTargetMode mode) {
  sendCC((uint8_t)target, 74, lfoTargetModeToCcValue(mode));
}

bool arpControlsEnabledForTarget(TargetChannel target) {
  return target == TARGET_P1 || target == TARGET_P2 || target == TARGET_TRI;
}

void sendArpSyncForTarget(TargetChannel target, bool enabled) {
  if (!arpControlsEnabledForTarget(target)) {
    return;
  }
  sendCC((uint8_t)target, 25, enabled ? 127 : 0);
}

void clearGlideForTargetIfNeeded(TargetChannel target) {
  if (!arpControlsEnabledForTarget(target)) {
    return;
  }
  if (!currentArpSyncEnabled) {
    return;
  }
  sendCC((uint8_t)target, 5, 0);
}

bool resolveControlMapping(TargetChannel target, ControlIndex controlIndex, uint8_t midiValue, uint8_t* outCc, uint8_t* outValue) {
  if (controlIndex == CTRL_ATTACK) {
    *outCc = 73;
    *outValue = midiValue;
    return true;
  }
  if (controlIndex == CTRL_DECAY) {
    *outCc = 75;
    *outValue = midiValue;
    return true;
  }
  if (controlIndex == CTRL_SUSTAIN) {
    *outCc = 71;
    *outValue = midiValue;
    return true;
  }
  if (controlIndex == CTRL_RELEASE) {
    *outCc = 72;
    *outValue = midiValue;
    return true;
  }

  switch (target) {
    case TARGET_P1:
    case TARGET_P2:
      if (controlIndex == CTRL_1) { *outCc = currentArpSyncEnabled ? 27 : 5;  *outValue = currentArpSyncEnabled ? quantizeArpSpeed(midiValue, true) : midiValue; return true; }
      if (controlIndex == CTRL_2) { *outCc = 16; *outValue = quantizeDuty4(midiValue); return true; }
      if (controlIndex == CTRL_3) { *outCc = 1;  *outValue = midiValue; return true; }
      if (controlIndex == CTRL_4) { *outCc = 76; *outValue = midiValue; return true; }
      return false;

    case TARGET_TRI:
      if (controlIndex == CTRL_1) { *outCc = currentArpSyncEnabled ? 27 : 5;  *outValue = currentArpSyncEnabled ? quantizeArpSpeed(midiValue, true) : midiValue; return true; }
      if (controlIndex == CTRL_2) { *outCc = 28; *outValue = quantizeOnOff(midiValue); return true; }
      if (controlIndex == CTRL_3) { *outCc = 1;  *outValue = midiValue; return true; }
      if (controlIndex == CTRL_4) { *outCc = 76; *outValue = midiValue; return true; }
      return false;

    case TARGET_NOISE:
      if (controlIndex == CTRL_1) { *outCc = 16; *outValue = midiValue; return true; }
      if (controlIndex == CTRL_2) { *outCc = 1;  *outValue = midiValue; return true; }
      return false;

    case TARGET_GLOBAL:
      if (controlIndex == CTRL_1) { *outCc = 5;  *outValue = midiValue; return true; }
      if (controlIndex == CTRL_2) { *outCc = 16; *outValue = quantizeDuty4(midiValue); return true; }
      if (controlIndex == CTRL_3) { *outCc = 1;  *outValue = midiValue; return true; }
      if (controlIndex == CTRL_4) { *outCc = 76; *outValue = midiValue; return true; }
      return false;
  }

  return false;
}

void sendAllControlsForTarget(TargetChannel target, bool forceSend = false) {
  for (uint8_t i = 0; i < CTRL_COUNT; ++i) {
    if (!CONTROL_ENABLED[i]) {
      continue;
    }
    uint8_t midiValue = mapRawToMidi(filteredAnalogRaw[i]);
    midiValue = normalizeMidiForControl((ControlIndex)i, midiValue);
    uint8_t cc = 0;
    uint8_t mappedValue = 0;
    if (resolveControlMapping(target, (ControlIndex)i, midiValue, &cc, &mappedValue)) {
      pendingMappedValue[i] = mappedValue;
      pendingMappedCount[i] = stableScansRequiredForControl((ControlIndex)i);
      if (forceSend || mappedValue != lastSentMappedValue[i]) {
        logControlEvent((ControlIndex)i, target, cc, mappedValue);
        sendCC((uint8_t)target, cc, mappedValue);
        lastSentMappedValue[i] = mappedValue;
      }
    }
    lastSentValue[i] = midiValue;
  }
}

void sendSingleControlForTarget(TargetChannel target, ControlIndex controlIndex, bool forceSend = false) {
  uint8_t i = (uint8_t)controlIndex;
  if (!CONTROL_ENABLED[i]) {
    return;
  }

  uint8_t midiValue = mapRawToMidi(filteredAnalogRaw[i]);
  midiValue = normalizeMidiForControl(controlIndex, midiValue);

  uint8_t cc = 0;
  uint8_t mappedValue = 0;
  if (resolveControlMapping(target, controlIndex, midiValue, &cc, &mappedValue)) {
    pendingMappedValue[i] = mappedValue;
    pendingMappedCount[i] = stableScansRequiredForControl(controlIndex);
    if (forceSend || mappedValue != lastSentMappedValue[i]) {
      logControlEvent(controlIndex, target, cc, mappedValue);
      sendCC((uint8_t)target, cc, mappedValue);
      lastSentMappedValue[i] = mappedValue;
    }
  }

  lastSentValue[i] = midiValue;
}

void setupControls() {
  analogReference(DEFAULT);

  for (uint8_t i = 0; i < SEL_COUNT; ++i) {
    pinMode(SELECTOR_PINS[i], INPUT_PULLUP);
  }
  pinMode(ARP_SYNC_PIN, INPUT_PULLUP);
  for (uint8_t i = 0; i < 3; ++i) {
    pinMode(LFO_TARGET_PINS[i], INPUT_PULLUP);
  }

#if defined(DIDR0)
  DIDR0 |= _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D) | _BV(ADC3D) | _BV(ADC4D) | _BV(ADC5D);
#endif
#if defined(DIDR1) && defined(ADC6D) && defined(ADC7D)
  DIDR1 |= _BV(ADC6D) | _BV(ADC7D);
#endif

  for (uint8_t i = 0; i < CTRL_COUNT; ++i) {
    if (CONTROL_ENABLED[i]) {
      lastAnalogRaw[i] = readStableAnalog(CONTROL_PINS[i]);
      filteredAnalogRaw[i] = lastAnalogRaw[i];
      lastSentValue[i] = mapRawToMidi(filteredAnalogRaw[i]);
    } else {
      lastAnalogRaw[i] = 0;
      filteredAnalogRaw[i] = 0;
      lastSentValue[i] = 0;
    }
    lastSentMappedValue[i] = 0xFF;
    pendingMappedValue[i] = 0xFF;
    pendingMappedCount[i] = 0;
  }
}

void processControlSelectors() {
  TargetChannel rawTarget;
  if (readTargetSelector(&rawTarget)) {
    if (rawTarget == pendingTarget) {
      if (pendingTargetCount < 255) pendingTargetCount++;
    } else {
      pendingTarget = rawTarget;
      pendingTargetCount = 1;
    }

    if (pendingTargetCount >= SELECTOR_STABLE_SCANS && pendingTarget != currentTarget) {
      currentTarget = pendingTarget;
      lastTarget = currentTarget;
      for (uint8_t i = 0; i < CTRL_COUNT; ++i) {
        lastSentMappedValue[i] = 0xFF;
        pendingMappedValue[i] = 0xFF;
        pendingMappedCount[i] = 0;
      }
      Serial.print(F("# Target -> CH"));
      Serial.println((uint8_t)currentTarget);
      sendArpSyncForTarget(currentTarget, currentArpSyncEnabled);
      clearGlideForTargetIfNeeded(currentTarget);
      sendLfoTargetForTarget(currentTarget, currentLfoTarget);
      sendAllControlsForTarget(currentTarget, true);
    }
  }

  {
    bool rawArpSyncEnabled = readArpSyncSwitch();
    if (rawArpSyncEnabled == pendingArpSyncEnabled) {
      if (pendingArpSyncCount < 255) pendingArpSyncCount++;
    } else {
      pendingArpSyncEnabled = rawArpSyncEnabled;
      pendingArpSyncCount = 1;
    }

    if (pendingArpSyncCount >= SELECTOR_STABLE_SCANS && pendingArpSyncEnabled != currentArpSyncEnabled) {
      currentArpSyncEnabled = pendingArpSyncEnabled;
      lastSentMappedValue[CTRL_1] = 0xFF;
      pendingMappedValue[CTRL_1] = 0xFF;
      pendingMappedCount[CTRL_1] = 0;
      Serial.print(F("# Arp sync -> "));
      Serial.println(currentArpSyncEnabled ? F("ON") : F("OFF"));
      sendArpSyncForTarget(currentTarget, currentArpSyncEnabled);
      clearGlideForTargetIfNeeded(currentTarget);
      sendSingleControlForTarget(currentTarget, CTRL_1, true);
    }
  }

  LfoTargetMode rawLfoTarget;
  if (readLfoTargetSelector(&rawLfoTarget)) {
    if (rawLfoTarget == pendingLfoTarget) {
      if (pendingLfoTargetCount < 255) pendingLfoTargetCount++;
    } else {
      pendingLfoTarget = rawLfoTarget;
      pendingLfoTargetCount = 1;
    }

    if (pendingLfoTargetCount >= SELECTOR_STABLE_SCANS && pendingLfoTarget != currentLfoTarget) {
      currentLfoTarget = pendingLfoTarget;
      lastLfoTarget = currentLfoTarget;
      sendLfoTargetForTarget(currentTarget, currentLfoTarget);
    }
  }
}

void processOneControl() {
  uint8_t i = nextControlIndex;
  ControlIndex controlIndex = (ControlIndex)i;
  uint8_t cc = 0;
  uint8_t mappedValue = 0;

  nextControlIndex++;
  if (nextControlIndex >= CTRL_COUNT) {
    nextControlIndex = 0;
  }

  if (!CONTROL_ENABLED[i]) {
    return;
  }

  int raw = readStableAnalog(CONTROL_PINS[i]);
  lastAnalogRaw[i] = raw;
  filteredAnalogRaw[i] = filteredRawForControl(controlIndex, filteredAnalogRaw[i], raw);

  uint8_t midiValue = mapRawToMidi(filteredAnalogRaw[i]);
  midiValue = normalizeMidiForControl(controlIndex, midiValue);

  if (resolveControlMapping(currentTarget, controlIndex, midiValue, &cc, &mappedValue)) {
    if (mappedValue == pendingMappedValue[i]) {
      if (pendingMappedCount[i] < 255) pendingMappedCount[i]++;
    } else {
      pendingMappedValue[i] = mappedValue;
      pendingMappedCount[i] = 1;
    }

    if (pendingMappedCount[i] >= stableScansRequiredForControl(controlIndex) &&
        mappedValue != lastSentMappedValue[i]) {
      logControlEvent(controlIndex, currentTarget, cc, mappedValue);
      sendCC((uint8_t)currentTarget, cc, mappedValue);
      lastSentMappedValue[i] = mappedValue;
    }
  }

  lastSentValue[i] = midiValue;
}

void setup() {
  pinMode(MIDI_RX_PIN, INPUT_PULLUP);
  pinMode(MIDI_TX_PIN, OUTPUT);

  Serial.begin(USB_BAUD);
  midiSerial.begin(MIDI_BAUD);
  setupControls();
  delay(STARTUP_DELAY_MS);

  if (!readTargetSelector(&currentTarget)) {
    currentTarget = DEFAULT_TARGET;
  }
  lastTarget = currentTarget;
  pendingTarget = currentTarget;
  pendingTargetCount = SELECTOR_STABLE_SCANS;

  if (!readLfoTargetSelector(&currentLfoTarget)) {
    currentLfoTarget = LFO_TO_PITCH;
  }
  lastLfoTarget = currentLfoTarget;
  pendingLfoTarget = currentLfoTarget;
  pendingLfoTargetCount = SELECTOR_STABLE_SCANS;
  currentArpSyncEnabled = readArpSyncSwitch();
  pendingArpSyncEnabled = currentArpSyncEnabled;
  pendingArpSyncCount = SELECTOR_STABLE_SCANS;

  Serial.println(F("# NanoMidiControl ready"));
  Serial.println(F("# USB serial 115200"));
  Serial.println(F("# MIDI DIN IN on D2 at 31250"));
  Serial.println(F("# Controls + DIN -> bridge_separate_channels.py"));
  Serial.println(F("# Selector: P1/P2/TRI/GLOBAL + D7 Sync"));
  Serial.println(F("# LFO switch: Pitch/Duty/Amp"));

  sendArpSyncForTarget(currentTarget, currentArpSyncEnabled);
  clearGlideForTargetIfNeeded(currentTarget);
  sendLfoTargetForTarget(currentTarget, currentLfoTarget);
  sendAllControlsForTarget(currentTarget, true);
}

void loop() {
  while (midiSerial.available() > 0) {
    processMidiByte((uint8_t)midiSerial.read());
  }

  if ((millis() - lastControlScanMs) >= CONTROL_SCAN_MS) {
    lastControlScanMs = millis();
    processControlSelectors();
    processOneControl();
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

  if (!midiTrafficSeen && (millis() - lastRealtimeMs > 2000UL)) {
    lastRealtimeMs = millis();
    Serial.println(F("# Idle, waiting for DIN MIDI"));
  }
}
