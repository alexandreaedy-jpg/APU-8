#include <Arduino.h>

// Dedicated V2 test: audible P1 LFO only.
// Keeps P2 and TRI silent and sends clear pitch modulation to P1.

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
static const uint32_t HEARTBEAT_MS = 2000U;

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

struct ProtoEvent {
  uint8_t regId;
  uint8_t value;
};

static const uint8_t kArmSequence[] = {0x04, 0x02, 0x01, 0x07};
static const uint8_t QUEUE_CAPACITY = 16;
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
static uint8_t lastAckOut = 0xFF;
static bool sawFirstAck = false;
static uint32_t lastHeartbeatMs = 0;
static uint32_t outCandidateSinceUs = 0;
static uint32_t lastLfoUpdateMs = 0;
static uint8_t lfoStep = 0;

static const uint8_t baseP1Note = 60;
static const uint8_t lfoDepth = 12;
static const uint32_t lfoStepMs = 50;
static const uint8_t lfoSteps = 16;

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
  const uint8_t pins[5] = {PIN_JOYPAD_D0, PIN_JOYPAD_D1, PIN_JOYPAD_D2, PIN_JOYPAD_D3, PIN_JOYPAD_D4};
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

static void resetQueue() {
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

static uint8_t buildStatusPattern() {
  const bool pending = (queueCount != 0);
  if (pending && overflowLatched) return STATUS_PENDING_OVERFLOW_VALID;
  if (pending) return STATUS_PENDING_VALID;
  if (overflowLatched) return STATUS_OVERFLOW_VALID;
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

static void seedInitialState() {
  resetQueue();
  enqueueEvent({REG_DUTY, 2});
  enqueueEvent({REG_ADSR_A, 4});
  enqueueEvent({REG_ADSR_D, 6});
  enqueueEvent({REG_ADSR_S, 10});
  enqueueEvent({REG_ADSR_R, 5});
  enqueueEvent({REG_P1_NOTE, baseP1Note});
  enqueueEvent({REG_P1_GATE, 1});
  enqueueEvent({REG_P1_TRIG, 1});
}

static void resetLiveSource() {
  sawFirstAck = false;
  lastLfoUpdateMs = millis();
  lfoStep = 0;
  seedInitialState();
}

static int16_t computeLfoOffset(void) {
  uint8_t pos = lfoStep;
  uint8_t half = lfoSteps / 2;
  int16_t level = (pos < half) ? pos : (lfoSteps - 1 - pos);
  int16_t offset = (level * lfoDepth * 2) / (half - 1) - lfoDepth;
  return offset;
}

static void enqueueLfoNoteUpdate(uint32_t nowMs) {
  if (!g245Enabled || !sawFirstAck) return;
  if ((uint32_t)(nowMs - lastLfoUpdateMs) < lfoStepMs) return;

  lastLfoUpdateMs = nowMs;
  lfoStep = (uint8_t)((lfoStep + 1) % lfoSteps);
  int16_t offset = computeLfoOffset();
  int16_t note = (int16_t)baseP1Note + offset;
  if (note < 0) note = 0;
  if (note > 127) note = 127;

  if (queueCount < QUEUE_CAPACITY) {
    enqueueEvent({REG_P1_NOTE, (uint8_t)note});
  }
}

static void updateReloadDetector(uint8_t out, bool outChanged) {
  if (!g245Enabled || !outChanged || (queueCount != 0)) return;

  static uint8_t reloadIndex = 0;
  if (out == kArmSequence[reloadIndex]) {
    ++reloadIndex;
    if (reloadIndex >= (sizeof(kArmSequence) / sizeof(kArmSequence[0]))) {
      reloadIndex = 0;
      resetLiveSource();
      writePattern(STATUS_VALID_ONLY);
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
      if (outChanged && lastAckOut != out) {
        popEvent();
        if (!sawFirstAck) sawFirstAck = true;
        lastAckOut = out;
      }
      nextPattern = buildStatusPattern();
      break;
    default:
      nextPattern = STATUS_VALID_ONLY;
      break;
  }
  if (out != OPCODE_ACK_AND_NEXT) lastAckOut = 0xFF;
  if (nextPattern != currentPattern) writePattern(nextPattern);
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
  Serial.println(F("PicoNesV2A_LFO_P1_Only start"));
  Serial.println(F("P1 only LFO modulation test"));
  currentOut = readOutBits();
  committedOut = currentOut;
  outCandidateSinceUs = micros();
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
  if ((currentOut != committedOut) && ((uint32_t)(nowUs - outCandidateSinceUs) >= OUT_SETTLE_US)) {
    committedOut = currentOut;
    out = committedOut;
    outChanged = true;
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
          Serial.println(F("245 enabled"));
          applyOpcode(out, outChanged);
        }
      } else if (out == kArmSequence[0]) {
        armIndex = 1;
      } else if (out != 0x00) {
        armIndex = 0;
      }
    }
  } else {
    enqueueLfoNoteUpdate(nowMs);
    updateReloadDetector(out, outChanged);
    applyOpcode(out, outChanged);
  }

  if ((uint32_t)(nowMs - lastHeartbeatMs) >= HEARTBEAT_MS) {
    lastHeartbeatMs = nowMs;
    Serial.print(F("step="));
    Serial.print(lfoStep);
    Serial.print(F(" q="));
    Serial.println(queueCount);
  }
}
