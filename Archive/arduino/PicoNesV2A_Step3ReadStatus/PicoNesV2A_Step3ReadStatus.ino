#include <Arduino.h>

// V2_A / Step 3 READ_STATUS-only probe
//
// Goal:
// - keep the 74HCT245 disabled until the ROM explicitly unlocks it
// - after unlock, decode only one opcode: READ_STATUS
// - when READ_STATUS is requested, return:
//     D0 = 1 (PENDING)
//     D4 = 1 (VALID)
//   so the logical readback is 0b10001
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

static const uint8_t STATUS_NONE = 0x00;
static const uint8_t STATUS_PENDING_VALID = 0x11; // D4=1, D0=1

static const uint8_t kArmSequence[] = {
  0x04, // observed as OUT=100
  0x02, // observed as OUT=010
  0x01, // observed as OUT=001
  0x07  // observed as OUT=111
};

static bool g245Enabled = false;
static uint8_t currentPattern = STATUS_NONE;
static uint8_t currentOut = 0xFF;
static uint8_t armIndex = 0;
static uint32_t lastHeartbeatMs = 0;

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
  Serial.println(currentPattern & 1);
}

static void applyOpcode(uint8_t out) {
  uint8_t nextPattern = STATUS_NONE;

  if (out == OPCODE_READ_STATUS) {
    nextPattern = STATUS_PENDING_VALID;
  } else if (out == OPCODE_IDLE) {
    nextPattern = STATUS_NONE;
  } else {
    nextPattern = STATUS_NONE;
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
  writePattern(STATUS_NONE);

  Serial.begin(115200);
  delay(250);
  Serial.println();
  Serial.println(F("PicoNesV2A_Step3ReadStatus start"));
  Serial.println(F("Unlock sequence: 100 -> 010 -> 001 -> 111"));
  Serial.println(F("READ_STATUS response: D=10001"));

  currentOut = readOutBits();
  logState("boot");
}

void loop() {
  const uint32_t now = millis();
  const uint8_t out = readOutBits();
  const bool outChanged = (out != currentOut);

  if (outChanged) {
    currentOut = out;
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
          applyOpcode(out);
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
    applyOpcode(out);
  }

  if ((uint32_t)(now - lastHeartbeatMs) >= 1000U) {
    lastHeartbeatMs = now;
    logState("heartbeat");
  }
}
