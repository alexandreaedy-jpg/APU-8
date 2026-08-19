#include <Arduino.h>

// V2_A / Step 4 atomic queue probe
//
// Goal:
// - keep the 74HCT245 disabled until the ROM explicitly unlocks it
// - expose a tiny 2-event queue
// - keep the current event frozen until ACK_AND_NEXT
// - let the ROM read STATUS / REG_ID / VALUE_LO / VALUE_HI cleanly
//
// Log format:
// - OUT=xyz means OUT2 OUT1 OUT0
// - D=abcde means D4 D3 D2 D1 D0

static const uint8_t PIN_OUT0 = 2;
static const uint8_t PIN_OUT1 = 3;
static const uint8_t PIN_OUT2 = 4;

static const uint8_t PIN_245_OE = 7; // active low

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
static const uint8_t OPCODE_READ_VALUE_LO_OBSERVED = 0x06; // observed phase on current wiring/log path

static const uint8_t STATUS_VALID_ONLY = 0x10;    // VALID=1, PENDING=0
static const uint8_t STATUS_PENDING_VALID = 0x11; // VALID=1, PENDING=1
static const uint32_t OUT_SETTLE_US = 1200U;

static const uint8_t kArmSequence[] = {
  0x04, // observed as OUT=100
  0x02, // observed as OUT=010
  0x01, // observed as OUT=001
  0x07  // observed as OUT=111
};

struct ProtoEvent {
  uint8_t regId;
  uint8_t value;
};

static const ProtoEvent kQueueTemplate[] = {
  {0x0, 0x3C}, // DUTY = 0x3C
  {0x5, 0xA7}, // LFO_RATE = 0xA7
};

static bool g245Enabled = false;
static uint8_t currentPattern = STATUS_VALID_ONLY;
static uint8_t currentOut = 0xFF;      // raw candidate
static uint8_t committedOut = 0xFF;    // stable opcode actually consumed
static uint8_t armIndex = 0;
static uint8_t reloadIndex = 0;
static uint32_t lastHeartbeatMs = 0;
static uint32_t outCandidateSinceUs = 0;
static uint8_t queueHead = 0;
static uint8_t queueCount = (uint8_t)(sizeof(kQueueTemplate) / sizeof(kQueueTemplate[0]));
static uint8_t lastAckOut = 0xFF;

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
  // Joypad readback is active-low at the NES side.
  digitalWrite(PIN_JOYPAD_D0, (pattern & 0x01U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D1, (pattern & 0x02U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D2, (pattern & 0x04U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D3, (pattern & 0x08U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D4, (pattern & 0x10U) ? LOW : HIGH);
  currentPattern = pattern;
}

static void printQueueState() {
  Serial.print(F(" | Q="));
  Serial.print(queueCount);
  Serial.print(F(" | E="));
  if (queueCount == 0) {
    Serial.print(F("--"));
    Serial.print(F(" V="));
    Serial.print(F("--"));
  } else {
    const ProtoEvent &evt = kQueueTemplate[queueHead];
    if (evt.regId < 16) {
      Serial.print(evt.regId, HEX);
    } else {
      Serial.print(F("??"));
    }
    Serial.print(F(" V="));
    if (evt.value < 16) {
      Serial.print('0');
    }
    Serial.print(evt.value, HEX);
  }
}

static void logState(const char* reason) {
  const uint8_t out = readOutBits();

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
  return (queueCount == 0) ? STATUS_VALID_ONLY : STATUS_PENDING_VALID;
}

static uint8_t buildRegIdPattern() {
  if (queueCount == 0) {
    return STATUS_VALID_ONLY;
  }
  return (uint8_t)(0x10U | (kQueueTemplate[queueHead].regId & 0x0FU));
}

static uint8_t buildValueLoPattern() {
  if (queueCount == 0) {
    return STATUS_VALID_ONLY;
  }
  return (uint8_t)(0x10U | (kQueueTemplate[queueHead].value & 0x0FU));
}

static uint8_t buildValueHiPattern() {
  if (queueCount == 0) {
    return STATUS_VALID_ONLY;
  }
  return (uint8_t)(0x10U | ((kQueueTemplate[queueHead].value >> 4) & 0x0FU));
}

static void resetQueue() {
  queueHead = 0;
  queueCount = (uint8_t)(sizeof(kQueueTemplate) / sizeof(kQueueTemplate[0]));
}

static bool popEvent() {
  if (queueCount == 0) {
    return false;
  }

  ++queueHead;
  if (queueHead >= (sizeof(kQueueTemplate) / sizeof(kQueueTemplate[0]))) {
    queueHead = 0;
  }
  --queueCount;
  return true;
}

static void updateReloadDetector(uint8_t out, bool outChanged) {
  if (!g245Enabled || !outChanged) {
    return;
  }

  if (out == kArmSequence[reloadIndex]) {
    ++reloadIndex;

    if (reloadIndex >= (sizeof(kArmSequence) / sizeof(kArmSequence[0]))) {
      reloadIndex = 0;
      resetQueue();
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
  writePattern(STATUS_VALID_ONLY);

  Serial.begin(115200);
  delay(250);
  Serial.println();
  Serial.println(F("PicoNesV2A_Step4AtomicQueue start"));
  Serial.println(F("Unlock sequence: 100 -> 010 -> 001 -> 111"));
  Serial.println(F("Events: E0 reg=0x0 val=0x3C ; E1 reg=0x5 val=0xA7"));

  currentOut = readOutBits();
  committedOut = currentOut;
  outCandidateSinceUs = micros();
  logState("boot");
}

void loop() {
  const uint32_t now = millis();
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
          logState("245 enabled");
          applyOpcode(out, outChanged);
        } else {
          logState("arm step");
        }
      } else if (out == kArmSequence[0]) {
        armIndex = 1;
        logState("arm restart");
      } else if (out == 0x00) {
        // stay quiet
      } else {
        armIndex = 0;
      }
    }
  } else {
    updateReloadDetector(out, outChanged);
    applyOpcode(out, outChanged);
  }

  if ((uint32_t)(now - lastHeartbeatMs) >= 1000U) {
    lastHeartbeatMs = now;
    logState("heartbeat");
  }
}
