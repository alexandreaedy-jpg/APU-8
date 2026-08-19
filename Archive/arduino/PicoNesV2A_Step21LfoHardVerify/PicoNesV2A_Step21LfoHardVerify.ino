#include <Arduino.h>

// V2_A / Step 21
// Hard LFO verification ROM support for:
// - P1 / P2 / TRI / NOISE / DMC samples
// - global duty + ADSR
// Goal: prepare MIDI/control input by separating musical state from transport.

static const uint8_t PIN_OUT0 = 2;
static const uint8_t PIN_OUT1 = 3;
static const uint8_t PIN_OUT2 = 4;

static const uint8_t PIN_245_OE = 7;

static const uint8_t PIN_JOYPAD_D0 = 10;
static const uint8_t PIN_JOYPAD_D1 = 11;
static const uint8_t PIN_JOYPAD_D2 = 12;
static const uint8_t PIN_JOYPAD_D3 = 13;
static const uint8_t PIN_JOYPAD_D4 = 14;

static const uint8_t OPCODE_IDLE = 0x00;
static const uint8_t OPCODE_READ_STATUS = 0x01;
static const uint8_t OPCODE_READ_REG_ID = 0x02;
static const uint8_t OPCODE_READ_VALUE_LO = 0x03;
static const uint8_t OPCODE_READ_VALUE_HI = 0x04;
static const uint8_t OPCODE_ACK_AND_NEXT = 0x05;
static const uint8_t OPCODE_READ_VALUE_LO_OBSERVED = 0x06;

static const uint8_t STATUS_VALID_ONLY = 0x10;
static const uint8_t STATUS_PENDING_VALID = 0x11;
static const uint8_t STATUS_PENDING_OVERFLOW_VALID = 0x13;
static const uint8_t STATUS_OVERFLOW_VALID = 0x12;

static const uint32_t OUT_SETTLE_US = 1200U;
static const uint32_t CONTROL_GENERATE_MS = 1000U;
static const uint32_t HEARTBEAT_MS = 4000U;

static const uint8_t kArmSequence[] = {0x04, 0x02, 0x01, 0x07};

struct ProtoEvent {
  uint8_t regId;
  uint8_t value;
};

struct ControlState {
  uint8_t p1Note;
  uint8_t p2Note;
  uint8_t triNote;
  uint8_t p1Gate;
  uint8_t p2Gate;
  uint8_t triGate;
  uint8_t p1Trig;
  uint8_t p2Trig;
  uint8_t triTrig;
  uint8_t attack;
  uint8_t decay;
  uint8_t sustain;
  uint8_t release;
  uint8_t duty;
  uint8_t lfoDepth;
  uint8_t lfoRate;
  uint8_t noiseNote;
  uint8_t noiseGate;
};

static const uint8_t REG_DUTY = 0x0;
static const uint8_t REG_ADSR_A = 0x1;
static const uint8_t REG_ADSR_D = 0x2;
static const uint8_t REG_ADSR_S = 0x3;
static const uint8_t REG_ADSR_R = 0x4;
static const uint8_t REG_TRI_NOTE = 0x5;
static const uint8_t REG_TRI_GATE = 0x6;
static const uint8_t REG_TRI_TRIG = 0x7;
static const uint8_t REG_P1_NOTE = 0x8;
static const uint8_t REG_P1_GATE = 0x9;
static const uint8_t REG_P1_TRIG = 0xA;
static const uint8_t REG_P2_NOTE = 0xB;
static const uint8_t REG_P2_GATE = 0xC;
static const uint8_t REG_P2_TRIG = 0xD;
static const uint8_t REG_NOI_NOTE = 0xE;
static const uint8_t REG_NOI_GATE = 0xF;

static inline uint8_t packDutyLfo(uint8_t duty, uint8_t depth, uint8_t rate) {
  return (uint8_t)((duty & 0x03U) | ((depth & 0x07U) << 2) | ((rate & 0x07U) << 5));
}

static const ControlState kInitialState = {
  40, 47, 40,
  1, 0, 0,
  1, 1, 1,
  0, 0, 15, 0,
  2, 7, 7,
  24, 0
};

static const ControlState kSongStates[] = {
  {40, 47, 40, 1, 0, 0, 2, 1, 1, 0, 0, 15, 0, 2, 7, 7, 24, 0},
  {40, 47, 40, 1, 0, 0, 3, 1, 1, 0, 0, 15, 0, 2, 7, 7, 24, 0}
};

static const uint8_t QUEUE_CAPACITY = 20;
static ProtoEvent queueBuf[QUEUE_CAPACITY];
static uint8_t queueHead = 0;
static uint8_t queueTail = 0;
static uint8_t queueCount = 0;

static bool g245Enabled = false;
static bool overflowLatched = false;
static uint8_t currentPattern = STATUS_VALID_ONLY;
static uint8_t currentOut = 0xFF;
static uint8_t committedOut = 0xFF;
static uint8_t armIndex = 0;
static uint8_t reloadIndex = 0;
static uint8_t lastAckOut = 0xFF;
static uint8_t stateIndex = 0;
static ControlState currentControlState = kInitialState;
static bool sawFirstAck = false;
static uint32_t lastHeartbeatMs = 0;
static uint32_t outCandidateSinceUs = 0;
static uint32_t lastGenerateMs = 0;

static inline uint8_t readOutBits() {
  return (uint8_t)((digitalRead(PIN_OUT0) ? 1 : 0) |
                   (digitalRead(PIN_OUT1) ? 2 : 0) |
                   (digitalRead(PIN_OUT2) ? 4 : 0));
}

static void disable245Safe() {
  pinMode(PIN_245_OE, OUTPUT);
  digitalWrite(PIN_245_OE, HIGH);
}

static void enable245() {
  digitalWrite(PIN_245_OE, LOW);
}

static void setupJoypadOutputs() {
  const uint8_t pins[5] = {
    PIN_JOYPAD_D0, PIN_JOYPAD_D1, PIN_JOYPAD_D2, PIN_JOYPAD_D3, PIN_JOYPAD_D4
  };
  for (uint8_t i = 0; i < 5; ++i) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
  }
}

static void setupOutInputs() {
  pinMode(PIN_OUT0, INPUT);
  pinMode(PIN_OUT1, INPUT);
  pinMode(PIN_OUT2, INPUT);
}

static void writePattern(uint8_t pattern) {
  digitalWrite(PIN_JOYPAD_D0, (pattern & 0x01U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D1, (pattern & 0x02U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D2, (pattern & 0x04U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D3, (pattern & 0x08U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D4, (pattern & 0x10U) ? LOW : HIGH);
  currentPattern = pattern;
}

static const ProtoEvent* frontEvent() {
  return (queueCount == 0) ? nullptr : &queueBuf[queueHead];
}

static void printQueueState() {
  Serial.print(F(" | Q="));
  Serial.print(queueCount);
  Serial.print(F(" | OVF="));
  Serial.print(overflowLatched ? 1 : 0);
  Serial.print(F(" | E="));
  const ProtoEvent* evt = frontEvent();
  if (!evt) {
    Serial.print(F("--"));
    Serial.print(F(" V="));
    Serial.print(F("--"));
  } else {
    Serial.print(evt->regId, HEX);
    Serial.print(F(" V="));
    if (evt->value < 16) {
      Serial.print('0');
    }
    Serial.print(evt->value, HEX);
  }
}

static void logState(const char* reason) {
  const uint8_t out = committedOut;

  Serial.print('[');
  Serial.print(millis());
  Serial.print(F(" ms] "));
  Serial.print(reason);
  Serial.print(F(" | 245="));
  Serial.print(g245Enabled ? F("ON") : F("SAFE"));
  Serial.print(F(" | OUT="));
  Serial.print((out >> 2) & 1);
  Serial.print((out >> 1) & 1);
  Serial.print(out & 1);
  Serial.print(F(" | D="));
  Serial.print((currentPattern >> 4) & 1);
  Serial.print((currentPattern >> 3) & 1);
  Serial.print((currentPattern >> 2) & 1);
  Serial.print((currentPattern >> 1) & 1);
  Serial.print(currentPattern & 1);
  printQueueState();
  Serial.println();
}

static uint8_t buildStatusPattern() {
  const bool pending = (queueCount != 0);
  if (pending && overflowLatched) {
    return STATUS_PENDING_OVERFLOW_VALID;
  }
  if (pending) {
    return STATUS_PENDING_VALID;
  }
  if (overflowLatched) {
    return STATUS_OVERFLOW_VALID;
  }
  return STATUS_VALID_ONLY;
}

static uint8_t buildRegIdPattern() {
  const ProtoEvent* evt = frontEvent();
  return evt ? (uint8_t)(0x10U | (evt->regId & 0x0FU)) : STATUS_VALID_ONLY;
}

static uint8_t buildValueLoPattern() {
  const ProtoEvent* evt = frontEvent();
  return evt ? (uint8_t)(0x10U | (evt->value & 0x0FU)) : STATUS_VALID_ONLY;
}

static uint8_t buildValueHiPattern() {
  const ProtoEvent* evt = frontEvent();
  return evt ? (uint8_t)(0x10U | ((evt->value >> 4) & 0x0FU)) : STATUS_VALID_ONLY;
}

static void clearQueue() {
  queueHead = 0;
  queueTail = 0;
  queueCount = 0;
  overflowLatched = false;
}

static bool enqueueEvent(const ProtoEvent& evt) {
  if (queueCount >= QUEUE_CAPACITY) {
    overflowLatched = true;
    return false;
  }
  queueBuf[queueTail] = evt;
  queueTail = (uint8_t)((queueTail + 1U) % QUEUE_CAPACITY);
  ++queueCount;
  return true;
}

static bool popEvent() {
  if (queueCount == 0) {
    return false;
  }
  queueHead = (uint8_t)((queueHead + 1U) % QUEUE_CAPACITY);
  --queueCount;
  if (queueCount == 0) {
    overflowLatched = false;
  }
  return true;
}

static inline uint8_t countChange(uint8_t oldValue, uint8_t newValue, bool force) {
  return (force || oldValue != newValue) ? 1U : 0U;
}

static uint8_t packedDutyLfoFromState(const ControlState& state) {
  return packDutyLfo(state.duty, state.lfoDepth, state.lfoRate);
}

static uint8_t countControlChanges(const ControlState& previous, const ControlState& next, bool force) {
  uint8_t count = 0;

  count += countChange(previous.attack, next.attack, force);
  count += countChange(previous.decay, next.decay, force);
  count += countChange(previous.sustain, next.sustain, force);
  count += countChange(previous.release, next.release, force);
  count += countChange(packedDutyLfoFromState(previous), packedDutyLfoFromState(next), force);

  count += countChange(previous.p1Note, next.p1Note, force);
  count += countChange(previous.p2Note, next.p2Note, force);
  count += countChange(previous.triNote, next.triNote, force);
  count += countChange(previous.noiseNote, next.noiseNote, force);

  count += countChange(previous.p1Gate, next.p1Gate, force);
  count += countChange(previous.p2Gate, next.p2Gate, force);
  count += countChange(previous.triGate, next.triGate, force);
  count += countChange(previous.noiseGate, next.noiseGate, force);

  count += countChange(previous.p1Trig, next.p1Trig, force);
  count += countChange(previous.p2Trig, next.p2Trig, force);
  count += countChange(previous.triTrig, next.triTrig, force);

  return count;
}

static bool enqueueIfChanged(uint8_t regId, uint8_t previous, uint8_t next, bool force) {
  if (!force && previous == next) {
    return true;
  }
  const ProtoEvent evt = {regId, next};
  return enqueueEvent(evt);
}

static bool enqueueControlState(const ControlState& previous, const ControlState& next, bool force) {
  if (!enqueueIfChanged(REG_ADSR_A, previous.attack, next.attack, force)) return false;
  if (!enqueueIfChanged(REG_ADSR_D, previous.decay, next.decay, force)) return false;
  if (!enqueueIfChanged(REG_ADSR_S, previous.sustain, next.sustain, force)) return false;
  if (!enqueueIfChanged(REG_ADSR_R, previous.release, next.release, force)) return false;
  if (!enqueueIfChanged(REG_DUTY, packedDutyLfoFromState(previous), packedDutyLfoFromState(next), force)) return false;

  if (!enqueueIfChanged(REG_P1_NOTE, previous.p1Note, next.p1Note, force)) return false;
  if (!enqueueIfChanged(REG_P2_NOTE, previous.p2Note, next.p2Note, force)) return false;
  if (!enqueueIfChanged(REG_TRI_NOTE, previous.triNote, next.triNote, force)) return false;
  if (!enqueueIfChanged(REG_NOI_NOTE, previous.noiseNote, next.noiseNote, force)) return false;

  if (!enqueueIfChanged(REG_P1_GATE, previous.p1Gate, next.p1Gate, force)) return false;
  if (!enqueueIfChanged(REG_P2_GATE, previous.p2Gate, next.p2Gate, force)) return false;
  if (!enqueueIfChanged(REG_TRI_GATE, previous.triGate, next.triGate, force)) return false;
  if (!enqueueIfChanged(REG_NOI_GATE, previous.noiseGate, next.noiseGate, force)) return false;

  if (!enqueueIfChanged(REG_P1_TRIG, previous.p1Trig, next.p1Trig, force)) return false;
  if (!enqueueIfChanged(REG_P2_TRIG, previous.p2Trig, next.p2Trig, force)) return false;
  if (!enqueueIfChanged(REG_TRI_TRIG, previous.triTrig, next.triTrig, force)) return false;

  return true;
}

static void seedQueue() {
  clearQueue();
  currentControlState = kInitialState;
  enqueueControlState(kInitialState, kInitialState, true);
  stateIndex = 0;
}

static void resetLiveSource() {
  stateIndex = 0;
  seedQueue();
  sawFirstAck = false;
  lastGenerateMs = millis();
}

static void maybeGenerateControlState(uint32_t nowMs) {
  if (!g245Enabled || !sawFirstAck) {
    return;
  }

  while ((uint32_t)(nowMs - lastGenerateMs) >= CONTROL_GENERATE_MS) {
    const ControlState& nextState = kSongStates[stateIndex];
    const uint8_t needed = countControlChanges(currentControlState, nextState, false);

    if (queueCount > (uint8_t)(QUEUE_CAPACITY - needed)) {
      break;
    }

    lastGenerateMs += CONTROL_GENERATE_MS;

    if (enqueueControlState(currentControlState, nextState, false)) {
      currentControlState = nextState;
      logState("enqueue");
      stateIndex = (uint8_t)((stateIndex + 1U) % (sizeof(kSongStates) / sizeof(kSongStates[0])));
    } else {
      logState("overflow");
      break;
    }
  }
}

static void updateReloadDetector(uint8_t out, bool outChanged) {
  if (!g245Enabled || !outChanged || (queueCount != 0)) {
    return;
  }

  if (out == kArmSequence[reloadIndex]) {
    ++reloadIndex;
    if (reloadIndex >= (sizeof(kArmSequence) / sizeof(kArmSequence[0]))) {
      reloadIndex = 0;
      resetLiveSource();
      writePattern(STATUS_VALID_ONLY);
      logState("queue reload");
    }
  } else if (out == kArmSequence[0]) {
    reloadIndex = 1;
  } else {
    reloadIndex = 0;
  }
}

static void applyOpcode(uint8_t out, bool outChanged) {
  uint8_t nextPattern = currentPattern;

  switch (out) {
    case OPCODE_IDLE:
      nextPattern = STATUS_VALID_ONLY;
      break;
    case OPCODE_READ_STATUS:
      nextPattern = buildStatusPattern();
      break;
    case OPCODE_READ_REG_ID:
      nextPattern = buildRegIdPattern();
      break;
    case OPCODE_READ_VALUE_LO:
    case OPCODE_READ_VALUE_LO_OBSERVED:
      nextPattern = buildValueLoPattern();
      break;
    case OPCODE_READ_VALUE_HI:
      nextPattern = buildValueHiPattern();
      break;
    case OPCODE_ACK_AND_NEXT:
      nextPattern = buildStatusPattern();
      if (outChanged) {
        if (lastAckOut != out) {
          if (popEvent()) {
            if (!sawFirstAck) {
              sawFirstAck = true;
              lastGenerateMs = millis();
            }
            logState("ACK pop");
          } else {
            logState("ACK empty");
          }
        }
        lastAckOut = out;
      }
      nextPattern = buildStatusPattern();
      break;
    default:
      nextPattern = STATUS_VALID_ONLY;
      break;
  }

  if (out != OPCODE_ACK_AND_NEXT) {
    lastAckOut = 0xFF;
  }

  if (nextPattern != currentPattern) {
    writePattern(nextPattern);
    logState("D change");
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  disable245Safe();
  setupJoypadOutputs();
  setupOutInputs();
  resetLiveSource();
  writePattern(STATUS_VALID_ONLY);

  Serial.begin(115200);
  delay(250);
  Serial.println();
  Serial.println(F("PicoNesV2A_Step21LfoHardVerify start"));
  Serial.println(F("P1-only hard ROM pitch modulation"));

  currentOut = readOutBits();
  committedOut = currentOut;
  outCandidateSinceUs = micros();
  logState("boot");
}

void loop() {
  const uint32_t nowMs = millis();
  const uint32_t nowUs = micros();
  const uint8_t sampledOut = readOutBits();

  if (sampledOut != currentOut) {
    currentOut = sampledOut;
    outCandidateSinceUs = nowUs;
  }

  bool outChanged = false;
  uint8_t out = committedOut;
  if ((currentOut != committedOut) &&
      ((uint32_t)(nowUs - outCandidateSinceUs) >= OUT_SETTLE_US)) {
    committedOut = currentOut;
    out = committedOut;
    outChanged = true;
    logState("OUT change");
  }

  if (!g245Enabled) {
    if (outChanged) {
      if (out == kArmSequence[armIndex]) {
        ++armIndex;
        if (armIndex >= (sizeof(kArmSequence) / sizeof(kArmSequence[0]))) {
          g245Enabled = true;
          enable245();
          digitalWrite(LED_BUILTIN, HIGH);
          resetLiveSource();
          logState("245 enabled");
          applyOpcode(out, outChanged);
        } else {
          logState("arm step");
        }
      } else if (out == kArmSequence[0]) {
        armIndex = 1;
        logState("arm restart");
      } else if (out != 0x00) {
        armIndex = 0;
      }
    }
  } else {
    maybeGenerateControlState(nowMs);
    updateReloadDetector(out, outChanged);
    applyOpcode(out, outChanged);
  }

  if ((uint32_t)(nowMs - lastHeartbeatMs) >= HEARTBEAT_MS) {
    lastHeartbeatMs = nowMs;
    logState("heartbeat");
  }
}
