#include <Arduino.h>

// V2_A / Step 11
// Hardware NOISE lane proof for:
// - NOISE note / gate / trigger

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
static const uint32_t CONTROL_GENERATE_MS = 1800U;
static const uint32_t HEARTBEAT_MS = 4000U;

static const uint8_t kArmSequence[] = {0x04, 0x02, 0x01, 0x07};

struct ProtoEvent {
  uint8_t regId;
  uint8_t value;
};

struct ProtoBurst {
  uint8_t count;
  ProtoEvent events[3];
};

static const uint8_t REG_NOI_NOTE = 0xE;
static const uint8_t REG_NOI_GATE = 0xF;
static const uint8_t REG_NOI_TRIG = 0x4;

static const ProtoEvent kSeedEvents[] = {
  {REG_NOI_NOTE, 48},
  {REG_NOI_GATE, 1},
  {REG_NOI_TRIG, 1}
};

static const ProtoBurst kBurstSequence[] = {
  {2, {{REG_NOI_NOTE, 52}, {REG_NOI_TRIG, 2}, {0, 0}}},
  {2, {{REG_NOI_NOTE, 60}, {REG_NOI_TRIG, 3}, {0, 0}}},
  {2, {{REG_NOI_NOTE, 72}, {REG_NOI_TRIG, 4}, {0, 0}}},
  {1, {{REG_NOI_TRIG, 5}, {0, 0}, {0, 0}}},
  {1, {{REG_NOI_GATE, 0}, {0, 0}, {0, 0}}},
  {3, {{REG_NOI_NOTE, 84}, {REG_NOI_GATE, 1}, {REG_NOI_TRIG, 6}}},
  {2, {{REG_NOI_NOTE, 48}, {REG_NOI_TRIG, 7}, {0, 0}}},
  {1, {{REG_NOI_TRIG, 8}, {0, 0}, {0, 0}}},
  {1, {{REG_NOI_GATE, 0}, {0, 0}, {0, 0}}}
};

static const uint8_t QUEUE_CAPACITY = 12;
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
static uint8_t burstIndex = 0;
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

static bool enqueueBurst(const ProtoBurst& burst) {
  for (uint8_t i = 0; i < burst.count; ++i) {
    if (!enqueueEvent(burst.events[i])) {
      return false;
    }
  }
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

static void seedQueue() {
  clearQueue();
  const uint8_t eventCount = (uint8_t)(sizeof(kSeedEvents) / sizeof(kSeedEvents[0]));
  const uint8_t seedCount = (QUEUE_CAPACITY < eventCount) ? QUEUE_CAPACITY : eventCount;
  for (uint8_t i = 0; i < seedCount; ++i) {
    enqueueEvent(kSeedEvents[i]);
  }
  burstIndex = 0;
}

static void resetLiveSource() {
  burstIndex = 0;
  seedQueue();
  sawFirstAck = false;
  lastGenerateMs = millis();
}

static void maybeGenerateBurst(uint32_t nowMs) {
  if (!g245Enabled || !sawFirstAck) {
    return;
  }

  while ((uint32_t)(nowMs - lastGenerateMs) >= CONTROL_GENERATE_MS) {
    const ProtoBurst& burst = kBurstSequence[burstIndex];
    lastGenerateMs += CONTROL_GENERATE_MS;

    if (queueCount > (uint8_t)(QUEUE_CAPACITY - burst.count)) {
      break;
    }

    if (enqueueBurst(burst)) {
      logState("enqueue");
      burstIndex = (uint8_t)((burstIndex + 1U) % (sizeof(kBurstSequence) / sizeof(kBurstSequence[0])));
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
  Serial.println(F("PicoNesV2A_Step11NoiseNoteQueue start"));
  Serial.println(F("NOISE note/gate/trigger only"));

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
    maybeGenerateBurst(nowMs);
    updateReloadDetector(out, outChanged);
    applyOpcode(out, outChanged);
  }

  if ((uint32_t)(nowMs - lastHeartbeatMs) >= HEARTBEAT_MS) {
    lastHeartbeatMs = nowMs;
    logState("heartbeat");
  }
}
