#include <Arduino.h>
#include <avr/interrupt.h>
#include <string.h>


/*
  NanoNesControl
  --------------
  Firmware dedie au vrai hardware NES.

  Ce sketch remplace le bridge PC :
  - MIDI DIN IN sur D0 (UART materiel)
  - controles hardware locaux
  - generation directe du payload 58 octets
  - emission du payload via le port manette NES

  Cablage :

  MIDI DIN :
  - optocoupleur out -> D0
  - D1 reste la TX UART materielle (non utilisee ici)

  NES :
  - D3  <- CLK depuis la NES
  - D12 <- OUT/LATCH depuis la NES
  - D2  -> D0 vers la NES

  Selecteur de destination :
  - D4 -> P1
  - D5 -> P2
  - D6 -> TRI
  - D8 -> GLOBAL

  Switch ARP sync :
  - D7 -> GND = sync ON pour P1/P2/TRI

  Switch destination LFO :
  - D9  -> Pitch
  - D10 -> Duty
  - D11 -> Amp

  Analogiques :
  - A0 -> Attack
  - A1 -> Decay
  - A2 -> Sustain
  - A3 -> Release
  - A4 -> Glide ou Arp speed/division quand sync est ON
  - A5 -> Duty / Punch
  - A6 -> LFO depth
  - A7 -> LFO rate
*/

static const uint8_t MIDI_RX_PIN = 0;
static const uint8_t MIDI_TX_PIN = 1;

static const uint8_t NES_CLK_PIN = 3;
static const uint8_t NES_LATCH_PIN = 12;
static const uint8_t NES_DATA_PIN = 2;
static const bool NES_PORT_ENABLED = true;
static const uint8_t NES_ARM_PULSES = 3;
static const unsigned long NES_START_HIGH_US = 2500;
static const unsigned long NES_TRANSFER_TIMEOUT_US = 100000;

static const unsigned long MIDI_BAUD = 31250;
static const unsigned long STARTUP_DELAY_MS = 250;
static const unsigned long CONTROL_SCAN_MS = 2;
static const unsigned long NOISE_MIN_GATE_HOLD_MS = 35;
static const uint8_t ANALOG_READ_SAMPLES = 3;
static const uint8_t SELECTOR_STABLE_SCANS = 4;
static const uint8_t MAX_STACK_NOTES = 8;
static const uint8_t PACKET_SIZE = 58;
static const bool FORCE_FIXED_NES_PAYLOAD = true;

static const bool DEBUG_USB = false;
static const unsigned long USB_BAUD = 115200;

enum TargetChannel : uint8_t {
  TARGET_P1 = 12,
  TARGET_P2 = 13,
  TARGET_TRI = 14,
  TARGET_NOISE = 15,
  TARGET_GLOBAL = 16
};

enum VoiceId : uint8_t {
  VOICE_P1 = 0,
  VOICE_P2 = 1,
  VOICE_TRI = 2,
  VOICE_NOI = 3,
  VOICE_COUNT = 4
};

enum SelectorIndex : uint8_t {
  SEL_P1 = 0,
  SEL_P2 = 1,
  SEL_TRI = 2,
  SEL_GLOBAL = 3,
  SEL_COUNT = 4
};

enum LfoTargetMode : uint8_t {
  LFO_TO_PITCH = 0,
  LFO_TO_DUTY = 1,
  LFO_TO_AMP = 2
};

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

enum ArpMode : uint8_t {
  ARP_OFF = 0,
  ARP_UP = 1,
  ARP_DOWN = 2,
  ARP_UPDOWN = 3
};

enum ParamKey : uint8_t {
  PARAM_ATTACK = 0,
  PARAM_DECAY = 1,
  PARAM_SUSTAIN = 2,
  PARAM_RELEASE = 3,
  PARAM_VIB_DEPTH = 4,
  PARAM_VIB_RATE = 5,
  PARAM_LFO_TARGET = 6,
  PARAM_GLIDE = 7,
  PARAM_TIMBRE = 8,
  PARAM_MODE = 9,
  PARAM_DUTY = 10,
  PARAM_TRI_PUNCH = 11
};

struct NoteEntry {
  uint8_t note;
  uint8_t vel;
};

struct NoteStack {
  uint8_t count;
  NoteEntry entries[MAX_STACK_NOTES];
};

struct VoiceState {
  uint8_t note;
  uint8_t vel;
  uint8_t gate;
  uint8_t trigger;
};

struct VoiceParams {
  uint8_t duty;
  uint8_t glide;
  uint8_t vibDepth;
  uint8_t vibRate;
  uint8_t lfoTarget;
  uint8_t attack;
  uint8_t decay;
  uint8_t sustain;
  uint8_t release;
  uint8_t timbre;
  uint8_t mode;
  uint8_t punch;
};

struct ArpState {
  bool enabled;
  bool sync;
  uint8_t mode;
  uint8_t speedCc;
  uint8_t phase;
  int8_t dir;
  unsigned long lastStepMs;
  uint8_t clockCounter;
};

static const uint8_t SELECTOR_PINS[SEL_COUNT] = {4, 5, 6, 8};
static const TargetChannel DEFAULT_TARGET = TARGET_P2;
static const uint8_t ARP_SYNC_PIN = 7;
static const uint8_t LFO_TARGET_PINS[3] = {9, 10, 11};
static const uint8_t CONTROL_PINS[CTRL_COUNT] = {A0, A1, A2, A3, A4, A5, A6, A7};
static const bool CONTROL_ENABLED[CTRL_COUNT] = {
  true, true, true, true, true, true, true, true
};

static const uint8_t DEFAULT_NOTES[VOICE_COUNT] = {60, 64, 48, 36};
static const uint8_t DEFAULT_VEL = 100;

volatile uint8_t activePacket[PACKET_SIZE];
volatile uint8_t latchedPacket[PACKET_SIZE];
volatile uint8_t shiftByteIndex = PACKET_SIZE;
volatile uint8_t shiftBitMask = 1;
volatile bool nesLatchHigh = true;
volatile bool nesSawOutRise = false;
volatile bool nesArmed = false;
volatile bool nesTransferActive = false;
volatile bool nesPortActive = false;
volatile uint8_t nesArmCount = 0;
volatile unsigned long nesOutRiseUs = 0;
volatile unsigned long nesLastEdgeUs = 0;

uint8_t busSeq = 0;
uint8_t busAttack = 0;
uint8_t busDecay = 32;
uint8_t busSustain = 96;
uint8_t busRelease = 24;

VoiceState voices[VOICE_COUNT];
VoiceParams voiceParams[VOICE_COUNT];
NoteStack localStacks[VOICE_COUNT];
NoteStack globalStack;
ArpState localArps[3];
ArpState globalArp;

bool midiClockRunning = false;
bool noiPendingNoteOff = false;
uint8_t noiPendingNoteOffNote = 0;
unsigned long noiGateHoldUntilMs = 0;

int lastAnalogRaw[CTRL_COUNT];
int filteredAnalogRaw[CTRL_COUNT];
uint8_t lastSentValue[CTRL_COUNT];
uint8_t lastSentMappedValue[CTRL_COUNT];
uint8_t pendingMappedValue[CTRL_COUNT];
uint8_t pendingMappedCount[CTRL_COUNT];

TargetChannel currentTarget = DEFAULT_TARGET;
LfoTargetMode currentLfoTarget = LFO_TO_PITCH;
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

inline void debugLine(const __FlashStringHelper* line) {
  if (DEBUG_USB) {
    Serial.println(line);
  }
}

inline void setNesDataBit(bool high) {
  DDRD |= _BV(PD2);
  if (high) {
    PORTD |= _BV(PD2);
  } else {
    PORTD &= (uint8_t)~_BV(PD2);
  }
}

inline void releaseNesDataBit() {
  PORTD &= (uint8_t)~_BV(PD2);
  DDRD &= (uint8_t)~_BV(PD2);
  nesTransferActive = false;
}

inline void resetNesArmState() {
  nesArmed = false;
  nesArmCount = 0;
}

inline void outputCurrentLatchedBit() {
  if (shiftByteIndex >= PACKET_SIZE) {
    setNesDataBit(0);
    return;
  }

  setNesDataBit((latchedPacket[shiftByteIndex] & shiftBitMask) != 0);
}

inline void beginControllerTransfer() {
  nesTransferActive = true;
  nesArmed = false;
  nesArmCount = 0;
  for (uint8_t i = 0; i < PACKET_SIZE; ++i) {
    latchedPacket[i] = activePacket[i];
  }
  shiftByteIndex = 0;
  shiftBitMask = 1;
  outputCurrentLatchedBit();
}

inline void advanceControllerBit() {
  if (shiftByteIndex >= PACKET_SIZE) {
    shiftByteIndex = 0;
    shiftBitMask = 1;
  }

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
    nesOutRiseUs = now;
    nesLastEdgeUs = now;
    nesSawOutRise = true;
  } else if (!latchHigh && nesLatchHigh) {
    if (nesSawOutRise) {
      const unsigned long highUs = now - nesOutRiseUs;
      nesLastEdgeUs = now;
      nesSawOutRise = false;
      if (highUs >= NES_START_HIGH_US) {
        beginControllerTransfer();
      } else if (nesTransferActive) {
        advanceControllerBit();
      }
    }
  }

  nesLatchHigh = latchHigh;
}

void serviceNesTransferTimeout() {
  bool active;
  bool armed;
  uint8_t armCount;
  unsigned long lastEdge;

  noInterrupts();
  active = nesTransferActive;
  armed = nesArmed;
  armCount = nesArmCount;
  lastEdge = nesLastEdgeUs;
  interrupts();

  if ((active || armed || armCount > 0) &&
      lastEdge != 0 &&
      (unsigned long)(micros() - lastEdge) >= NES_TRANSFER_TIMEOUT_US) {
    noInterrupts();
    shiftByteIndex = PACKET_SIZE;
    releaseNesDataBit();
    resetNesArmState();
    nesSawOutRise = false;
    interrupts();
  }
}

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

  return (uint8_t)(((unsigned int)(value - lowDeadzone) * 127U + ((highClamp - lowDeadzone) / 2U)) /
                   (unsigned int)(highClamp - lowDeadzone));
}

uint8_t zeroClampForControl(ControlIndex controlIndex, uint8_t value) {
  uint8_t threshold = 0;

  switch (controlIndex) {
    case CTRL_ATTACK: threshold = 14; break;
    case CTRL_DECAY: threshold = 8; break;
    case CTRL_SUSTAIN: threshold = 8; break;
    case CTRL_RELEASE: threshold = 16; break;
    case CTRL_1: threshold = 10; break;
    case CTRL_2: threshold = 4; break;
    case CTRL_3: threshold = 10; break;
    case CTRL_4: threshold = 10; break;
    default: threshold = 0; break;
  }

  if (value <= threshold) {
    return 0;
  }
  return value;
}

uint8_t normalizeMidiForControl(ControlIndex controlIndex, uint8_t value) {
  value = zeroClampForControl(controlIndex, value);

  if (controlIndex == CTRL_ATTACK) return stretchMidiRange(value, 18, 122);
  if (controlIndex == CTRL_DECAY) return stretchMidiRange(value, 8, 120);
  if (controlIndex == CTRL_SUSTAIN) return stretchMidiRange(value, 8, 123);
  if (controlIndex == CTRL_RELEASE) return stretchMidiRange(value, 14, 120);
  if (controlIndex == CTRL_1) return stretchMidiRange(value, 8, 123);
  if (controlIndex == CTRL_2) return stretchMidiRange(value, 4, 123);
  if (controlIndex == CTRL_3) return stretchMidiRange(value, 18, 123);
  if (controlIndex == CTRL_4) return quantizeVibratoRate(stretchMidiRange(value, 6, 123));
  return value;
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
  if (controlIndex <= CTRL_RELEASE) return 1;
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

bool arpControlsEnabledForTarget(TargetChannel target) {
  return target == TARGET_P1 || target == TARGET_P2 || target == TARGET_TRI;
}

uint16_t arpIntervalMsFromCc(uint8_t speedCc) {
  return (uint16_t)(24U + (((uint16_t)(127U - speedCc) * 310U) / 127U));
}

uint8_t arpClockTicksFromCc(uint8_t speedCc) {
  if (speedCc >= 112) return 3;
  if (speedCc >= 96) return 4;
  if (speedCc >= 80) return 6;
  if (speedCc >= 64) return 8;
  if (speedCc >= 48) return 12;
  if (speedCc >= 32) return 16;
  if (speedCc >= 16) return 24;
  return 48;
}

void reverseNotes(NoteEntry* entries, uint8_t count) {
  for (uint8_t i = 0; i < (count / 2); ++i) {
    NoteEntry tmp = entries[i];
    entries[i] = entries[count - 1 - i];
    entries[count - 1 - i] = tmp;
  }
}

uint8_t buildOrderedStack(const NoteStack& stack, uint8_t mode, NoteEntry* out) {
  uint8_t count = stack.count;

  for (uint8_t i = 0; i < count; ++i) {
    out[i] = stack.entries[i];
  }

  for (uint8_t i = 0; i < count; ++i) {
    for (uint8_t j = (uint8_t)(i + 1); j < count; ++j) {
      if (out[j].note < out[i].note) {
        NoteEntry tmp = out[i];
        out[i] = out[j];
        out[j] = tmp;
      }
    }
  }

  if (mode == ARP_DOWN) {
    reverseNotes(out, count);
  }

  return count;
}

void advanceArpPhase(uint8_t notes, ArpState* arp) {
  if (notes <= 1) {
    arp->phase = 0;
    arp->dir = 1;
    return;
  }

  if (arp->mode == ARP_UP) {
    arp->phase = (uint8_t)((arp->phase + 1) % notes);
    arp->dir = 1;
    return;
  }

  if (arp->mode == ARP_DOWN) {
    arp->phase = (uint8_t)((arp->phase + 1) % notes);
    arp->dir = -1;
    return;
  }

  {
    int16_t next = (int16_t)arp->phase + arp->dir;
    if (next >= notes) {
      arp->dir = -1;
      next = notes - 2;
    } else if (next < 0) {
      arp->dir = 1;
      next = 1;
    }
    arp->phase = (uint8_t)next;
  }
}

void buildFixedNesPayload() {
  uint8_t packet[PACKET_SIZE];
  memset(packet, 0, sizeof(packet));

  packet[0] = 1;
  packet[8] = 0xA8;
  packet[9] = 0x58;

  packet[13] = 64;
  packet[14] = 110;
  packet[15] = 1;

  packet[26] = 2;
  packet[27] = 0;
  packet[28] = 0;
  packet[29] = 0;
  packet[37] = 0;
  packet[38] = 0;
  packet[39] = 127;
  packet[40] = 0;
  packet[53] = 1;

  noInterrupts();
  memcpy((void*)activePacket, packet, PACKET_SIZE);
  interrupts();
}

void rebuildPayload() {
  uint8_t packet[PACKET_SIZE];
  uint8_t i = 0;

  packet[i++] = busSeq;
  packet[i++] = busAttack;
  packet[i++] = busDecay;
  packet[i++] = busSustain;
  packet[i++] = busRelease;
  packet[i++] = voiceParams[VOICE_P1].lfoTarget;
  packet[i++] = voiceParams[VOICE_P2].lfoTarget;
  packet[i++] = voiceParams[VOICE_TRI].lfoTarget;
  packet[i++] = 0xA8;
  packet[i++] = 0x58;

  packet[i++] = voices[VOICE_P1].note;
  packet[i++] = voices[VOICE_P1].vel;
  packet[i++] = voices[VOICE_P1].gate;
  packet[i++] = voices[VOICE_P2].note;
  packet[i++] = voices[VOICE_P2].vel;
  packet[i++] = voices[VOICE_P2].gate;
  packet[i++] = voices[VOICE_TRI].note;
  packet[i++] = voices[VOICE_TRI].vel;
  packet[i++] = voices[VOICE_TRI].gate;
  packet[i++] = voices[VOICE_NOI].note;
  packet[i++] = voices[VOICE_NOI].vel;
  packet[i++] = voices[VOICE_NOI].gate;

  packet[i++] = voiceParams[VOICE_P1].duty;
  packet[i++] = voiceParams[VOICE_P1].glide;
  packet[i++] = voiceParams[VOICE_P1].vibDepth;
  packet[i++] = voiceParams[VOICE_P1].vibRate;
  packet[i++] = voiceParams[VOICE_P2].duty;
  packet[i++] = voiceParams[VOICE_P2].glide;
  packet[i++] = voiceParams[VOICE_P2].vibDepth;
  packet[i++] = voiceParams[VOICE_P2].vibRate;
  packet[i++] = voiceParams[VOICE_TRI].glide;
  packet[i++] = voiceParams[VOICE_TRI].vibDepth;
  packet[i++] = voiceParams[VOICE_TRI].vibRate;

  packet[i++] = voiceParams[VOICE_P1].attack;
  packet[i++] = voiceParams[VOICE_P1].decay;
  packet[i++] = voiceParams[VOICE_P1].sustain;
  packet[i++] = voiceParams[VOICE_P1].release;
  packet[i++] = voiceParams[VOICE_P2].attack;
  packet[i++] = voiceParams[VOICE_P2].decay;
  packet[i++] = voiceParams[VOICE_P2].sustain;
  packet[i++] = voiceParams[VOICE_P2].release;
  packet[i++] = voiceParams[VOICE_TRI].attack;
  packet[i++] = voiceParams[VOICE_TRI].decay;
  packet[i++] = voiceParams[VOICE_TRI].sustain;
  packet[i++] = voiceParams[VOICE_TRI].release;
  packet[i++] = voiceParams[VOICE_NOI].attack;
  packet[i++] = voiceParams[VOICE_NOI].decay;
  packet[i++] = voiceParams[VOICE_NOI].sustain;
  packet[i++] = voiceParams[VOICE_NOI].release;
  packet[i++] = voiceParams[VOICE_NOI].timbre;
  packet[i++] = voiceParams[VOICE_NOI].mode;
  packet[i++] = voiceParams[VOICE_TRI].punch;
  packet[i++] = voices[VOICE_P1].trigger;
  packet[i++] = voices[VOICE_P2].trigger;
  packet[i++] = voices[VOICE_TRI].trigger;
  packet[i++] = voices[VOICE_NOI].trigger;

  while (i < PACKET_SIZE) {
    packet[i++] = 0;
  }

  noInterrupts();
  memcpy((void*)activePacket, packet, PACKET_SIZE);
  interrupts();
}

void commitStateChange() {
  busSeq = (uint8_t)(busSeq + 1);
  rebuildPayload();
}

bool setVoiceOutput(uint8_t voiceId, uint8_t note, uint8_t vel, bool gate) {
  bool changed = false;

  if (voices[voiceId].note != note) {
    voices[voiceId].note = note;
    changed = true;
  }
  if (voices[voiceId].vel != vel) {
    voices[voiceId].vel = vel;
    changed = true;
  }
  if (voices[voiceId].gate != (gate ? 1 : 0)) {
    voices[voiceId].gate = gate ? 1 : 0;
    changed = true;
  }

  return changed;
}

void bumpVoiceTrigger(uint8_t voiceId) {
  voices[voiceId].trigger = (uint8_t)(voices[voiceId].trigger + 1);
}

bool setVoiceParam(uint8_t voiceId, ParamKey key, uint8_t value) {
  uint8_t* slot = 0;

  switch (key) {
    case PARAM_ATTACK:
      slot = &voiceParams[voiceId].attack;
      value &= 0x7F;
      break;
    case PARAM_DECAY:
      slot = &voiceParams[voiceId].decay;
      value &= 0x7F;
      break;
    case PARAM_SUSTAIN:
      slot = &voiceParams[voiceId].sustain;
      value &= 0x7F;
      break;
    case PARAM_RELEASE:
      slot = &voiceParams[voiceId].release;
      value &= 0x7F;
      break;
    case PARAM_VIB_DEPTH:
      slot = &voiceParams[voiceId].vibDepth;
      value &= 0x7F;
      break;
    case PARAM_VIB_RATE:
      slot = &voiceParams[voiceId].vibRate;
      value &= 0x7F;
      break;
    case PARAM_LFO_TARGET:
      if (voiceId > VOICE_TRI) {
        return false;
      }
      if (value < 43) value = LFO_TO_PITCH;
      else if (value < 86) value = LFO_TO_DUTY;
      else value = LFO_TO_AMP;
      slot = &voiceParams[voiceId].lfoTarget;
      break;
    case PARAM_GLIDE:
      if (voiceId == VOICE_NOI) {
        return false;
      }
      slot = &voiceParams[voiceId].glide;
      value &= 0x7F;
      break;
    case PARAM_TIMBRE:
      if (voiceId != VOICE_NOI) {
        return false;
      }
      slot = &voiceParams[voiceId].timbre;
      value &= 0x7F;
      break;
    case PARAM_MODE:
      if (voiceId != VOICE_NOI) {
        return false;
      }
      slot = &voiceParams[voiceId].mode;
      value = value >= 64 ? 127 : 0;
      break;
    case PARAM_DUTY:
      if (voiceId != VOICE_P1 && voiceId != VOICE_P2) {
        return false;
      }
      slot = &voiceParams[voiceId].duty;
      value &= 0x03;
      break;
    case PARAM_TRI_PUNCH:
      if (voiceId != VOICE_TRI) {
        return false;
      }
      slot = &voiceParams[voiceId].punch;
      value = value >= 64 ? 127 : 0;
      break;
  }

  if (*slot == value) {
    return false;
  }

  *slot = value;
  return true;
}

bool addNoteToStack(NoteStack* stack, uint8_t note, uint8_t vel) {
  uint8_t i;

  for (i = 0; i < stack->count; ++i) {
    if (stack->entries[i].note == note) {
      for (; i + 1 < stack->count; ++i) {
        stack->entries[i] = stack->entries[i + 1];
      }
      --stack->count;
      break;
    }
  }

  if (stack->count >= MAX_STACK_NOTES) {
    stack->count = MAX_STACK_NOTES - 1;
  }

  for (i = stack->count; i > 0; --i) {
    stack->entries[i] = stack->entries[i - 1];
  }

  stack->entries[0].note = note;
  stack->entries[0].vel = vel;
  ++stack->count;
  return true;
}

bool removeNoteFromStack(NoteStack* stack, uint8_t note) {
  for (uint8_t i = 0; i < stack->count; ++i) {
    if (stack->entries[i].note == note) {
      for (; i + 1 < stack->count; ++i) {
        stack->entries[i] = stack->entries[i + 1];
      }
      --stack->count;
      return true;
    }
  }
  return false;
}

bool clearNoteStack(NoteStack* stack) {
  if (stack->count == 0) {
    return false;
  }
  stack->count = 0;
  return true;
}

void resetArpTransport(uint8_t channel) {
  unsigned long now = millis();

  if (channel == TARGET_GLOBAL) {
    globalArp.lastStepMs = now;
    globalArp.clockCounter = 0;
    return;
  }
  if (channel == TARGET_P1) {
    localArps[VOICE_P1].lastStepMs = now;
    localArps[VOICE_P1].clockCounter = 0;
    return;
  }
  if (channel == TARGET_P2) {
    localArps[VOICE_P2].lastStepMs = now;
    localArps[VOICE_P2].clockCounter = 0;
    return;
  }
  if (channel == TARGET_TRI) {
    localArps[VOICE_TRI].lastStepMs = now;
    localArps[VOICE_TRI].clockCounter = 0;
  }
}

bool refreshLocalVoice(uint8_t voiceId) {
  NoteEntry ordered[MAX_STACK_NOTES];
  uint8_t orderedCount = 0;
  NoteStack* stack = &localStacks[voiceId];

  if (voiceId >= 3) {
    if (stack->count > 0) {
      return setVoiceOutput(voiceId, stack->entries[0].note, stack->entries[0].vel, true);
    }
    return setVoiceOutput(voiceId, voices[voiceId].note, voices[voiceId].vel, false);
  }

  if (localArps[voiceId].enabled && stack->count > 1) {
    orderedCount = buildOrderedStack(*stack, localArps[voiceId].mode, ordered);
    if (localArps[voiceId].phase >= orderedCount) {
      localArps[voiceId].phase = 0;
    }
    return setVoiceOutput(voiceId, ordered[localArps[voiceId].phase].note, ordered[localArps[voiceId].phase].vel, true);
  }

  if (stack->count > 0) {
    return setVoiceOutput(voiceId, stack->entries[0].note, stack->entries[0].vel, true);
  }

  return setVoiceOutput(voiceId, voices[voiceId].note, voices[voiceId].vel, false);
}

bool refreshGlobalSplit() {
  bool changed = false;
  NoteEntry ordered[MAX_STACK_NOTES];
  uint8_t orderedCount = 0;

  if (globalArp.enabled && globalStack.count > 0) {
    orderedCount = buildOrderedStack(globalStack, globalArp.mode, ordered);
    if (globalArp.phase >= orderedCount) {
      globalArp.phase = 0;
    }
    changed |= setVoiceOutput(VOICE_P1, ordered[globalArp.phase].note, ordered[globalArp.phase].vel, true);
    changed |= setVoiceOutput(VOICE_P2, voices[VOICE_P2].note, voices[VOICE_P2].vel, false);
    changed |= setVoiceOutput(VOICE_TRI, voices[VOICE_TRI].note, voices[VOICE_TRI].vel, false);
    return changed;
  }

  if (globalStack.count > 0) changed |= setVoiceOutput(VOICE_P1, globalStack.entries[0].note, globalStack.entries[0].vel, true);
  else changed |= setVoiceOutput(VOICE_P1, voices[VOICE_P1].note, voices[VOICE_P1].vel, false);

  if (globalStack.count > 1) changed |= setVoiceOutput(VOICE_P2, globalStack.entries[1].note, globalStack.entries[1].vel, true);
  else changed |= setVoiceOutput(VOICE_P2, voices[VOICE_P2].note, voices[VOICE_P2].vel, false);

  if (globalStack.count > 2) changed |= setVoiceOutput(VOICE_TRI, globalStack.entries[2].note, globalStack.entries[2].vel, true);
  else changed |= setVoiceOutput(VOICE_TRI, voices[VOICE_TRI].note, voices[VOICE_TRI].vel, false);

  return changed;
}

bool noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  bool changed = false;
  note &= 0x7F;
  velocity &= 0x7F;
  if (velocity == 0) velocity = 1;

  if (channel == TARGET_P1 || channel == TARGET_P2 || channel == TARGET_TRI || channel == TARGET_NOISE) {
    uint8_t voiceId = (channel == TARGET_P1) ? VOICE_P1 :
                      (channel == TARGET_P2) ? VOICE_P2 :
                      (channel == TARGET_TRI) ? VOICE_TRI : VOICE_NOI;

    bumpVoiceTrigger(voiceId);
    changed = true;

    if (voiceId == VOICE_NOI) {
      noiGateHoldUntilMs = millis() + NOISE_MIN_GATE_HOLD_MS;
      noiPendingNoteOff = false;
    }

    addNoteToStack(&localStacks[voiceId], note, velocity);
    changed |= refreshLocalVoice(voiceId);
    resetArpTransport(channel);
  } else if (channel == TARGET_GLOBAL) {
    addNoteToStack(&globalStack, note, velocity);
    changed = true;
    changed |= refreshGlobalSplit();
    resetArpTransport(channel);
  }

  if (changed) {
    commitStateChange();
  }
  return changed;
}

bool noteOff(uint8_t channel, uint8_t note) {
  bool changed = false;
  note &= 0x7F;

  if (channel == TARGET_P1 || channel == TARGET_P2 || channel == TARGET_TRI || channel == TARGET_NOISE) {
    uint8_t voiceId = (channel == TARGET_P1) ? VOICE_P1 :
                      (channel == TARGET_P2) ? VOICE_P2 :
                      (channel == TARGET_TRI) ? VOICE_TRI : VOICE_NOI;

    if (voiceId == VOICE_NOI && millis() < noiGateHoldUntilMs) {
      noiPendingNoteOff = true;
      noiPendingNoteOffNote = note;
      return false;
    }

    changed |= removeNoteFromStack(&localStacks[voiceId], note);
    changed |= refreshLocalVoice(voiceId);
    resetArpTransport(channel);
  } else if (channel == TARGET_GLOBAL) {
    changed |= removeNoteFromStack(&globalStack, note);
    changed |= refreshGlobalSplit();
    resetArpTransport(channel);
  }

  if (changed) {
    commitStateChange();
  }
  return changed;
}

bool allNotesOff() {
  bool changed = false;

  for (uint8_t i = 0; i < VOICE_COUNT; ++i) {
    changed |= clearNoteStack(&localStacks[i]);
    changed |= setVoiceOutput(i, voices[i].note, voices[i].vel, false);
  }
  changed |= clearNoteStack(&globalStack);

  noiPendingNoteOff = false;
  noiGateHoldUntilMs = 0;

  if (changed) {
    commitStateChange();
  }
  return changed;
}

void processPendingNoiseOff() {
  bool changed = false;

  if (!noiPendingNoteOff) {
    return;
  }
  if (millis() < noiGateHoldUntilMs) {
    return;
  }

  noiPendingNoteOff = false;
  changed |= removeNoteFromStack(&localStacks[VOICE_NOI], noiPendingNoteOffNote);
  changed |= refreshLocalVoice(VOICE_NOI);

  if (changed) {
    commitStateChange();
  }
}

bool setVoiceControlValue(uint8_t voiceId, ParamKey key, uint8_t value) {
  return setVoiceParam(voiceId, key, value);
}

bool broadcastVoiceControl(ParamKey key, uint8_t value) {
  bool changed = false;

  for (uint8_t voiceId = 0; voiceId < VOICE_COUNT; ++voiceId) {
    changed |= setVoiceControlValue(voiceId, key, value);
  }

  return changed;
}

bool broadcastPolyLfoControl(ParamKey key, uint8_t value) {
  bool changed = false;
  changed |= setVoiceControlValue(VOICE_P1, key, value);
  changed |= setVoiceControlValue(VOICE_P2, key, value);

  if (key == PARAM_VIB_DEPTH) {
    changed |= setVoiceControlValue(VOICE_TRI, key, value / 4);
  } else {
    changed |= setVoiceControlValue(VOICE_TRI, key, value);
  }

  return changed;
}

bool parseControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
  bool changed = false;

  if (cc == 120 || cc == 123) {
    return allNotesOff();
  }

  if (cc == 71) {
    if (channel == TARGET_GLOBAL) changed = broadcastVoiceControl(PARAM_SUSTAIN, value);
    else if (channel >= TARGET_P1 && channel <= TARGET_NOISE) changed = setVoiceControlValue((uint8_t)(channel - TARGET_P1), PARAM_SUSTAIN, value);
  } else if (cc == 72) {
    if (channel == TARGET_GLOBAL) changed = broadcastVoiceControl(PARAM_RELEASE, value);
    else if (channel >= TARGET_P1 && channel <= TARGET_NOISE) changed = setVoiceControlValue((uint8_t)(channel - TARGET_P1), PARAM_RELEASE, value);
  } else if (cc == 73) {
    if (channel == TARGET_GLOBAL) changed = broadcastVoiceControl(PARAM_ATTACK, value);
    else if (channel >= TARGET_P1 && channel <= TARGET_NOISE) changed = setVoiceControlValue((uint8_t)(channel - TARGET_P1), PARAM_ATTACK, value);
  } else if (cc == 75) {
    if (channel == TARGET_GLOBAL) changed = broadcastVoiceControl(PARAM_DECAY, value);
    else if (channel >= TARGET_P1 && channel <= TARGET_NOISE) changed = setVoiceControlValue((uint8_t)(channel - TARGET_P1), PARAM_DECAY, value);
  } else if (channel == TARGET_NOISE) {
    if (cc == 16) changed = setVoiceControlValue(VOICE_NOI, PARAM_TIMBRE, value);
    else if (cc == 1) changed = setVoiceControlValue(VOICE_NOI, PARAM_MODE, value);
  } else if (channel == TARGET_P1) {
    if (cc == 25) {
      bool newSync = value >= 64;
      if (localArps[VOICE_P1].sync != newSync) {
        localArps[VOICE_P1].sync = newSync;
        localArps[VOICE_P1].clockCounter = 0;
        resetArpTransport(TARGET_P1);
        changed = true;
      }
    } else if (cc == 26) {
      uint8_t newMode = (value < 43) ? ARP_UP : (value < 86 ? ARP_DOWN : ARP_UPDOWN);
      if (localArps[VOICE_P1].mode != newMode || localArps[VOICE_P1].phase != 0 || localArps[VOICE_P1].dir != 1) {
        localArps[VOICE_P1].mode = newMode;
        localArps[VOICE_P1].phase = 0;
        localArps[VOICE_P1].dir = 1;
        localArps[VOICE_P1].clockCounter = 0;
        changed = true;
        changed |= refreshLocalVoice(VOICE_P1);
        resetArpTransport(TARGET_P1);
      }
    } else if (cc == 27) {
      if (localArps[VOICE_P1].speedCc != value) {
        localArps[VOICE_P1].speedCc = value;
        resetArpTransport(TARGET_P1);
        changed = true;
      }
    } else if (cc == 1) {
      changed = setVoiceControlValue(VOICE_P1, PARAM_VIB_DEPTH, value);
    } else if (cc == 5) {
      changed = setVoiceControlValue(VOICE_P1, PARAM_GLIDE, value);
    } else if (cc == 16) {
      changed = setVoiceControlValue(VOICE_P1, PARAM_DUTY, (uint8_t)((value * 4U) / 128U));
    } else if (cc == 76) {
      changed = setVoiceControlValue(VOICE_P1, PARAM_VIB_RATE, value);
    } else if (cc == 74) {
      changed = setVoiceControlValue(VOICE_P1, PARAM_LFO_TARGET, value);
    }
  } else if (channel == TARGET_P2) {
    if (cc == 25) {
      bool newSync = value >= 64;
      if (localArps[VOICE_P2].sync != newSync) {
        localArps[VOICE_P2].sync = newSync;
        localArps[VOICE_P2].clockCounter = 0;
        resetArpTransport(TARGET_P2);
        changed = true;
      }
    } else if (cc == 26) {
      uint8_t newMode = (value < 43) ? ARP_UP : (value < 86 ? ARP_DOWN : ARP_UPDOWN);
      if (localArps[VOICE_P2].mode != newMode || localArps[VOICE_P2].phase != 0 || localArps[VOICE_P2].dir != 1) {
        localArps[VOICE_P2].mode = newMode;
        localArps[VOICE_P2].phase = 0;
        localArps[VOICE_P2].dir = 1;
        localArps[VOICE_P2].clockCounter = 0;
        changed = true;
        changed |= refreshLocalVoice(VOICE_P2);
        resetArpTransport(TARGET_P2);
      }
    } else if (cc == 27) {
      if (localArps[VOICE_P2].speedCc != value) {
        localArps[VOICE_P2].speedCc = value;
        resetArpTransport(TARGET_P2);
        changed = true;
      }
    } else if (cc == 1) {
      changed = setVoiceControlValue(VOICE_P2, PARAM_VIB_DEPTH, value);
    } else if (cc == 5) {
      changed = setVoiceControlValue(VOICE_P2, PARAM_GLIDE, value);
    } else if (cc == 16) {
      changed = setVoiceControlValue(VOICE_P2, PARAM_DUTY, (uint8_t)((value * 4U) / 128U));
    } else if (cc == 76) {
      changed = setVoiceControlValue(VOICE_P2, PARAM_VIB_RATE, value);
    } else if (cc == 74) {
      changed = setVoiceControlValue(VOICE_P2, PARAM_LFO_TARGET, value);
    }
  } else if (channel == TARGET_TRI) {
    if (cc == 25) {
      bool newSync = value >= 64;
      if (localArps[VOICE_TRI].sync != newSync) {
        localArps[VOICE_TRI].sync = newSync;
        localArps[VOICE_TRI].clockCounter = 0;
        resetArpTransport(TARGET_TRI);
        changed = true;
      }
    } else if (cc == 26) {
      uint8_t newMode = (value < 43) ? ARP_UP : (value < 86 ? ARP_DOWN : ARP_UPDOWN);
      if (localArps[VOICE_TRI].mode != newMode || localArps[VOICE_TRI].phase != 0 || localArps[VOICE_TRI].dir != 1) {
        localArps[VOICE_TRI].mode = newMode;
        localArps[VOICE_TRI].phase = 0;
        localArps[VOICE_TRI].dir = 1;
        localArps[VOICE_TRI].clockCounter = 0;
        changed = true;
        changed |= refreshLocalVoice(VOICE_TRI);
        resetArpTransport(TARGET_TRI);
      }
    } else if (cc == 27) {
      if (localArps[VOICE_TRI].speedCc != value) {
        localArps[VOICE_TRI].speedCc = value;
        resetArpTransport(TARGET_TRI);
        changed = true;
      }
    } else if (cc == 28) {
      changed = setVoiceControlValue(VOICE_TRI, PARAM_TRI_PUNCH, value);
    } else if (cc == 1) {
      changed = setVoiceControlValue(VOICE_TRI, PARAM_VIB_DEPTH, value);
    } else if (cc == 5) {
      changed = setVoiceControlValue(VOICE_TRI, PARAM_GLIDE, value);
    } else if (cc == 76) {
      changed = setVoiceControlValue(VOICE_TRI, PARAM_VIB_RATE, value);
    } else if (cc == 74) {
      changed = setVoiceControlValue(VOICE_TRI, PARAM_LFO_TARGET, value);
    }
  } else if (channel == TARGET_GLOBAL) {
    if (cc == 24) {
      bool newEnabled = value >= 64;
      if (globalArp.enabled != newEnabled || globalArp.phase != 0 || globalArp.dir != 1) {
        globalArp.enabled = newEnabled;
        globalArp.phase = 0;
        globalArp.dir = 1;
        globalArp.clockCounter = 0;
        changed = true;
        changed |= refreshGlobalSplit();
        resetArpTransport(TARGET_GLOBAL);
      }
    } else if (cc == 25) {
      bool newSync = value >= 64;
      if (globalArp.sync != newSync) {
        globalArp.sync = newSync;
        globalArp.clockCounter = 0;
        resetArpTransport(TARGET_GLOBAL);
        changed = true;
      }
    } else if (cc == 26) {
      uint8_t newMode = (value < 43) ? ARP_UP : (value < 86 ? ARP_DOWN : ARP_UPDOWN);
      if (globalArp.mode != newMode || globalArp.phase != 0 || globalArp.dir != 1) {
        globalArp.mode = newMode;
        globalArp.phase = 0;
        globalArp.dir = 1;
        globalArp.clockCounter = 0;
        changed = true;
        changed |= refreshGlobalSplit();
        resetArpTransport(TARGET_GLOBAL);
      }
    } else if (cc == 27) {
      if (globalArp.speedCc != value) {
        globalArp.speedCc = value;
        resetArpTransport(TARGET_GLOBAL);
        changed = true;
      }
    } else if (cc == 1) {
      changed = broadcastPolyLfoControl(PARAM_VIB_DEPTH, value);
    } else if (cc == 5) {
      changed = broadcastVoiceControl(PARAM_GLIDE, value);
    } else if (cc == 16) {
      changed = broadcastVoiceControl(PARAM_DUTY, (uint8_t)((value * 4U) / 128U));
    } else if (cc == 76) {
      changed = broadcastPolyLfoControl(PARAM_VIB_RATE, value);
    } else if (cc == 74) {
      changed = broadcastVoiceControl(PARAM_LFO_TARGET, value);
    }
  }

  if (changed) {
    commitStateChange();
  }
  return changed;
}

void handleMidiStart() {
  globalArp.phase = 0;
  globalArp.dir = 1;
  globalArp.clockCounter = 0;
  localArps[VOICE_P1].phase = 0;
  localArps[VOICE_P1].dir = 1;
  localArps[VOICE_P1].clockCounter = 0;
  localArps[VOICE_P2].phase = 0;
  localArps[VOICE_P2].dir = 1;
  localArps[VOICE_P2].clockCounter = 0;
  localArps[VOICE_TRI].phase = 0;
  localArps[VOICE_TRI].dir = 1;
  localArps[VOICE_TRI].clockCounter = 0;
  midiClockRunning = true;
  refreshGlobalSplit();
  refreshLocalVoice(VOICE_P1);
  refreshLocalVoice(VOICE_P2);
  refreshLocalVoice(VOICE_TRI);
  commitStateChange();
}

void handleMidiContinue() {
  midiClockRunning = true;
}

void handleMidiStop() {
  midiClockRunning = false;
}

void stepGlobalArp() {
  NoteEntry ordered[MAX_STACK_NOTES];
  uint8_t notes = buildOrderedStack(globalStack, globalArp.mode, ordered);
  if (!globalArp.enabled || notes <= 1) {
    return;
  }
  advanceArpPhase(notes, &globalArp);
  bumpVoiceTrigger(VOICE_P1);
  refreshGlobalSplit();
  commitStateChange();
}

void stepLocalArp(uint8_t voiceId) {
  NoteEntry ordered[MAX_STACK_NOTES];
  uint8_t notes = buildOrderedStack(localStacks[voiceId], localArps[voiceId].mode, ordered);
  if (!localArps[voiceId].enabled || notes <= 1) {
    return;
  }
  advanceArpPhase(notes, &localArps[voiceId]);
  bumpVoiceTrigger(voiceId);
  refreshLocalVoice(voiceId);
  commitStateChange();
}

void handleMidiClock() {
  if (!midiClockRunning) {
    return;
  }

  if (globalArp.enabled && globalArp.sync && globalStack.count > 1) {
    ++globalArp.clockCounter;
    if (globalArp.clockCounter >= arpClockTicksFromCc(globalArp.speedCc)) {
      globalArp.clockCounter = 0;
      stepGlobalArp();
    }
  }

  if (localArps[VOICE_P1].enabled && localArps[VOICE_P1].sync && localStacks[VOICE_P1].count > 1) {
    ++localArps[VOICE_P1].clockCounter;
    if (localArps[VOICE_P1].clockCounter >= arpClockTicksFromCc(localArps[VOICE_P1].speedCc)) {
      localArps[VOICE_P1].clockCounter = 0;
      stepLocalArp(VOICE_P1);
    }
  }

  if (localArps[VOICE_P2].enabled && localArps[VOICE_P2].sync && localStacks[VOICE_P2].count > 1) {
    ++localArps[VOICE_P2].clockCounter;
    if (localArps[VOICE_P2].clockCounter >= arpClockTicksFromCc(localArps[VOICE_P2].speedCc)) {
      localArps[VOICE_P2].clockCounter = 0;
      stepLocalArp(VOICE_P2);
    }
  }

  if (localArps[VOICE_TRI].enabled && localArps[VOICE_TRI].sync && localStacks[VOICE_TRI].count > 1) {
    ++localArps[VOICE_TRI].clockCounter;
    if (localArps[VOICE_TRI].clockCounter >= arpClockTicksFromCc(localArps[VOICE_TRI].speedCc)) {
      localArps[VOICE_TRI].clockCounter = 0;
      stepLocalArp(VOICE_TRI);
    }
  }
}

void processFreeRunningArps() {
  unsigned long now = millis();

  if (globalArp.enabled && globalStack.count > 1 && (!globalArp.sync || !midiClockRunning)) {
    if ((uint16_t)(now - globalArp.lastStepMs) >= arpIntervalMsFromCc(globalArp.speedCc)) {
      globalArp.lastStepMs = now;
      stepGlobalArp();
    }
  }

  if (localArps[VOICE_P1].enabled && localStacks[VOICE_P1].count > 1 && (!localArps[VOICE_P1].sync || !midiClockRunning)) {
    if ((uint16_t)(now - localArps[VOICE_P1].lastStepMs) >= arpIntervalMsFromCc(localArps[VOICE_P1].speedCc)) {
      localArps[VOICE_P1].lastStepMs = now;
      stepLocalArp(VOICE_P1);
    }
  }

  if (localArps[VOICE_P2].enabled && localStacks[VOICE_P2].count > 1 && (!localArps[VOICE_P2].sync || !midiClockRunning)) {
    if ((uint16_t)(now - localArps[VOICE_P2].lastStepMs) >= arpIntervalMsFromCc(localArps[VOICE_P2].speedCc)) {
      localArps[VOICE_P2].lastStepMs = now;
      stepLocalArp(VOICE_P2);
    }
  }

  if (localArps[VOICE_TRI].enabled && localStacks[VOICE_TRI].count > 1 && (!localArps[VOICE_TRI].sync || !midiClockRunning)) {
    if ((uint16_t)(now - localArps[VOICE_TRI].lastStepMs) >= arpIntervalMsFromCc(localArps[VOICE_TRI].speedCc)) {
      localArps[VOICE_TRI].lastStepMs = now;
      stepLocalArp(VOICE_TRI);
    }
  }
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
      noteOff(channel, data0 & 0x7F);
      break;
    case 0x90:
      if ((data1 & 0x7F) == 0) noteOff(channel, data0 & 0x7F);
      else noteOn(channel, data0 & 0x7F, data1 & 0x7F);
      break;
    case 0xB0:
      parseControlChange(channel, data0 & 0x7F, data1 & 0x7F);
      break;
  }
}

void processMidiByte(uint8_t b) {
  if (b == 0xF8) {
    handleMidiClock();
    return;
  }
  if (b == 0xFA) {
    handleMidiStart();
    return;
  }
  if (b == 0xFB) {
    handleMidiContinue();
    return;
  }
  if (b == 0xFC) {
    handleMidiStop();
    return;
  }
  if (b >= 0xF8) {
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

  dataBytes[dataIndex++] = b & 0x7F;

  {
    uint8_t needed = expectedDataBytes(runningStatus);
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

void applyLfoTargetForTarget(TargetChannel target, LfoTargetMode mode) {
  parseControlChange((uint8_t)target, 74, lfoTargetModeToCcValue(mode));
}

void applyArpSyncForTarget(TargetChannel target, bool enabled) {
  if (!arpControlsEnabledForTarget(target)) {
    return;
  }
  parseControlChange((uint8_t)target, 25, enabled ? 127 : 0);
}

void clearGlideForTargetIfNeeded(TargetChannel target) {
  if (!arpControlsEnabledForTarget(target)) {
    return;
  }
  if (!currentArpSyncEnabled) {
    return;
  }
  parseControlChange((uint8_t)target, 5, 0);
}

void applyAllControlsForTarget(TargetChannel target, bool forceSend) {
  for (uint8_t i = 0; i < CTRL_COUNT; ++i) {
    uint8_t midiValue;
    uint8_t cc = 0;
    uint8_t mappedValue = 0;

    if (!CONTROL_ENABLED[i]) {
      continue;
    }

    midiValue = mapRawToMidi(filteredAnalogRaw[i]);
    midiValue = normalizeMidiForControl((ControlIndex)i, midiValue);

    if (resolveControlMapping(target, (ControlIndex)i, midiValue, &cc, &mappedValue)) {
      pendingMappedValue[i] = mappedValue;
      pendingMappedCount[i] = stableScansRequiredForControl((ControlIndex)i);
      if (forceSend || mappedValue != lastSentMappedValue[i]) {
        parseControlChange((uint8_t)target, cc, mappedValue);
        lastSentMappedValue[i] = mappedValue;
      }
    }

    lastSentValue[i] = midiValue;
  }
}

void applySingleControlForTarget(TargetChannel target, ControlIndex controlIndex, bool forceSend) {
  uint8_t i = (uint8_t)controlIndex;
  uint8_t midiValue;
  uint8_t cc = 0;
  uint8_t mappedValue = 0;

  if (!CONTROL_ENABLED[i]) {
    return;
  }

  midiValue = mapRawToMidi(filteredAnalogRaw[i]);
  midiValue = normalizeMidiForControl(controlIndex, midiValue);

  if (resolveControlMapping(target, controlIndex, midiValue, &cc, &mappedValue)) {
    pendingMappedValue[i] = mappedValue;
    pendingMappedCount[i] = stableScansRequiredForControl(controlIndex);
    if (forceSend || mappedValue != lastSentMappedValue[i]) {
      parseControlChange((uint8_t)target, cc, mappedValue);
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
  LfoTargetMode rawLfoTarget;

  if (readTargetSelector(&rawTarget)) {
    if (rawTarget == pendingTarget) {
      if (pendingTargetCount < 255) pendingTargetCount++;
    } else {
      pendingTarget = rawTarget;
      pendingTargetCount = 1;
    }

    if (pendingTargetCount >= SELECTOR_STABLE_SCANS && pendingTarget != currentTarget) {
      currentTarget = pendingTarget;
      for (uint8_t i = 0; i < CTRL_COUNT; ++i) {
        lastSentMappedValue[i] = 0xFF;
        pendingMappedValue[i] = 0xFF;
        pendingMappedCount[i] = 0;
      }
      applyArpSyncForTarget(currentTarget, currentArpSyncEnabled);
      clearGlideForTargetIfNeeded(currentTarget);
      applyLfoTargetForTarget(currentTarget, currentLfoTarget);
      applyAllControlsForTarget(currentTarget, true);
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
      applyArpSyncForTarget(currentTarget, currentArpSyncEnabled);
      clearGlideForTargetIfNeeded(currentTarget);
      applySingleControlForTarget(currentTarget, CTRL_1, true);
    }
  }

  if (readLfoTargetSelector(&rawLfoTarget)) {
    if (rawLfoTarget == pendingLfoTarget) {
      if (pendingLfoTargetCount < 255) pendingLfoTargetCount++;
    } else {
      pendingLfoTarget = rawLfoTarget;
      pendingLfoTargetCount = 1;
    }

    if (pendingLfoTargetCount >= SELECTOR_STABLE_SCANS && pendingLfoTarget != currentLfoTarget) {
      currentLfoTarget = pendingLfoTarget;
      applyLfoTargetForTarget(currentTarget, currentLfoTarget);
    }
  }
}

void processOneControl() {
  uint8_t i = nextControlIndex;
  ControlIndex controlIndex = (ControlIndex)i;
  uint8_t midiValue;
  uint8_t cc = 0;
  uint8_t mappedValue = 0;
  int raw;

  nextControlIndex++;
  if (nextControlIndex >= CTRL_COUNT) {
    nextControlIndex = 0;
  }

  if (!CONTROL_ENABLED[i]) {
    return;
  }

  raw = readStableAnalog(CONTROL_PINS[i]);
  lastAnalogRaw[i] = raw;
  filteredAnalogRaw[i] = filteredRawForControl(controlIndex, filteredAnalogRaw[i], raw);

  midiValue = mapRawToMidi(filteredAnalogRaw[i]);
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
      parseControlChange((uint8_t)currentTarget, cc, mappedValue);
      lastSentMappedValue[i] = mappedValue;
    }
  }

  lastSentValue[i] = midiValue;
}

void setupVoiceState() {
  for (uint8_t i = 0; i < VOICE_COUNT; ++i) {
    voices[i].note = DEFAULT_NOTES[i];
    voices[i].vel = DEFAULT_VEL;
    voices[i].gate = 0;
    voices[i].trigger = 0;
    localStacks[i].count = 0;

    voiceParams[i].duty = 2;
    voiceParams[i].glide = 0;
    voiceParams[i].vibDepth = 0;
    voiceParams[i].vibRate = 0;
    voiceParams[i].lfoTarget = LFO_TO_PITCH;
    voiceParams[i].attack = 0;
    voiceParams[i].decay = 32;
    voiceParams[i].sustain = 96;
    voiceParams[i].release = 24;
    voiceParams[i].timbre = 0;
    voiceParams[i].mode = 0;
    voiceParams[i].punch = 0;
  }

  voiceParams[VOICE_TRI].punch = 127;
  globalStack.count = 0;

  globalArp.enabled = false;
  globalArp.sync = false;
  globalArp.mode = ARP_UP;
  globalArp.speedCc = 32;
  globalArp.phase = 0;
  globalArp.dir = 1;
  globalArp.lastStepMs = millis();
  globalArp.clockCounter = 0;

  for (uint8_t i = 0; i < 3; ++i) {
    localArps[i].enabled = true;
    localArps[i].sync = false;
    localArps[i].mode = ARP_UP;
    localArps[i].speedCc = 32;
    localArps[i].phase = 0;
    localArps[i].dir = 1;
    localArps[i].lastStepMs = millis();
    localArps[i].clockCounter = 0;
  }
}

void setupNesPort() {
  pinMode(NES_CLK_PIN, INPUT_PULLUP);
  pinMode(NES_LATCH_PIN, INPUT_PULLUP);
  pinMode(NES_DATA_PIN, OUTPUT);
  setNesDataBit(0);
  nesPortActive = true;

  nesLatchHigh = (PINB & _BV(PB4)) != 0;
  nesSawOutRise = false;
  nesOutRiseUs = 0;
  nesLastEdgeUs = 0;

  noInterrupts();
  EIFR |= _BV(INTF1);
  EIMSK &= (uint8_t)~_BV(INT1);
  PCICR |= _BV(PCIE0);
  PCMSK0 |= _BV(PCINT4);
  PCIFR |= _BV(PCIF0);
  interrupts();
}

void setup() {
  if (FORCE_FIXED_NES_PAYLOAD) {
    setupNesPort();
    buildFixedNesPayload();
    beginControllerTransfer();
    return;
  }

  pinMode(MIDI_RX_PIN, INPUT_PULLUP);
  pinMode(MIDI_TX_PIN, OUTPUT);

  Serial.begin(MIDI_BAUD);
  setupControls();
  setupVoiceState();
  setupNesPort();
  delay(STARTUP_DELAY_MS);

  if (!readTargetSelector(&currentTarget)) {
    currentTarget = DEFAULT_TARGET;
  }
  pendingTarget = currentTarget;
  pendingTargetCount = SELECTOR_STABLE_SCANS;

  if (!readLfoTargetSelector(&currentLfoTarget)) {
    currentLfoTarget = LFO_TO_PITCH;
  }
  pendingLfoTarget = currentLfoTarget;
  pendingLfoTargetCount = SELECTOR_STABLE_SCANS;

  currentArpSyncEnabled = readArpSyncSwitch();
  pendingArpSyncEnabled = currentArpSyncEnabled;
  pendingArpSyncCount = SELECTOR_STABLE_SCANS;

  rebuildPayload();
  applyArpSyncForTarget(currentTarget, currentArpSyncEnabled);
  clearGlideForTargetIfNeeded(currentTarget);
  applyLfoTargetForTarget(currentTarget, currentLfoTarget);
  applyAllControlsForTarget(currentTarget, true);
  beginControllerTransfer();

  debugLine(F("# NanoNesControl ready"));
}

void loop() {
  if (FORCE_FIXED_NES_PAYLOAD) {
    return;
  }

  while (Serial.available() > 0) {
    processMidiByte((uint8_t)Serial.read());
  }

  if ((millis() - lastControlScanMs) >= CONTROL_SCAN_MS) {
    lastControlScanMs = millis();
    processControlSelectors();
    processOneControl();
  }

  processPendingNoiseOff();
  processFreeRunningArps();
}
