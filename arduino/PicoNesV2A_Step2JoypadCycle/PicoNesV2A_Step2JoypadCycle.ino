#include <Arduino.h>

// V2_A / Step 2 Joypad return-path probe
//
// Goal:
// - keep the 74HCT245 disabled until the ROM explicitly arms it
// - arm only when OUT0..OUT2 present a deliberate unlock sequence
// - then drive a slow one-hot pattern on Joypad D0..D4
// - let a ROM read $4017 and turn the observed 5-bit value into audio
//
// Log format:
// - D=abcde means D4 D3 D2 D1 D0
// - OUT=xyz means OUT2 OUT1 OUT0

static const uint8_t PIN_OUT0 = 2;       // 74HC4050 output from NES OUT0
static const uint8_t PIN_OUT1 = 3;       // 74HC4050 output from NES OUT1
static const uint8_t PIN_OUT2 = 4;       // 74HC4050 output from NES OUT2

static const uint8_t PIN_245_OE = 7;     // 74HCT245 /OE, active low

static const uint8_t PIN_JOYPAD_D0 = 10; // Pico -> 74HCT245 -> NES Joypad D0
static const uint8_t PIN_JOYPAD_D1 = 11; // Pico -> 74HCT245 -> NES Joypad D1
static const uint8_t PIN_JOYPAD_D2 = 12; // Pico -> 74HCT245 -> NES Joypad D2
static const uint8_t PIN_JOYPAD_D3 = 13; // Pico -> 74HCT245 -> NES Joypad D3
static const uint8_t PIN_JOYPAD_D4 = 14; // Pico -> 74HCT245 -> NES Joypad D4

static const uint32_t PATTERN_HOLD_MS = 2000U;
static const uint32_t HEARTBEAT_MS = 1000U;

static const uint8_t kArmSequence[] = {
  // Follow the observed OUT display order on the validated hardware path:
  // 100 -> 010 -> 001 -> 111
  0x04, // first observed step
  0x02, // OUT1
  0x01, // third observed step
  0x07  // final arm code
};

static const uint8_t kPatterns[] = {
  0x01, // D0
  0x02, // D1
  0x04, // D2
  0x08, // D3
  0x10, // D4
  0x00  // quiet / idle gap
};

static bool g245Enabled = false;
static uint8_t currentPattern = 0x00;
static uint8_t patternIndex = 0;
static uint32_t lastPatternMs = 0;
static uint32_t lastHeartbeatMs = 0;
static uint8_t lastOut = 0xFF;
static uint8_t armIndex = 0;

static inline uint8_t readOutBits() {
  return (uint8_t)((digitalRead(PIN_OUT0) ? 1 : 0) |
                   (digitalRead(PIN_OUT1) ? 2 : 0) |
                   (digitalRead(PIN_OUT2) ? 4 : 0));
}

static void disable245Safe() {
  pinMode(PIN_245_OE, OUTPUT);
  digitalWrite(PIN_245_OE, HIGH); // HIGH = tri-state / safe
}

static void enable245() {
  digitalWrite(PIN_245_OE, LOW); // LOW = active
}

static void setupJoypadOutputs() {
  const uint8_t pins[5] = {
    PIN_JOYPAD_D0, PIN_JOYPAD_D1, PIN_JOYPAD_D2, PIN_JOYPAD_D3, PIN_JOYPAD_D4
  };

  for (uint8_t i = 0; i < 5; ++i) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

static void setupOutInputs() {
  pinMode(PIN_OUT0, INPUT);
  pinMode(PIN_OUT1, INPUT);
  pinMode(PIN_OUT2, INPUT);
}

static void writePattern(uint8_t pattern) {
  // Joypad input lines are inverted before reaching $4017:
  // physical LOW reads back as logical 1, physical HIGH as logical 0.
  digitalWrite(PIN_JOYPAD_D0, (pattern & 0x01U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D1, (pattern & 0x02U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D2, (pattern & 0x04U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D3, (pattern & 0x08U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D4, (pattern & 0x10U) ? LOW : HIGH);
  currentPattern = pattern;
}

static void logPattern(const char* reason) {
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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  disable245Safe();
  setupJoypadOutputs();
  setupOutInputs();
  writePattern(0x00);

  Serial.begin(115200);
  delay(250);
  Serial.println();
  Serial.println(F("PicoNesV2A_Step2JoypadCycle start"));
  Serial.println(F("74HCT245 stays SAFE until OUT transitions through 100 010 001 111"));
  Serial.println(F("When armed, it drives a one-hot D0..D4 loop"));
  Serial.println(F("Log bit order is D4 D3 D2 D1 D0"));

  lastOut = readOutBits();
  logPattern("boot");
}

void loop() {
  const uint32_t now = millis();
  const uint8_t out = readOutBits();
  const bool outChanged = (out != lastOut);

  if (outChanged) {
    lastOut = out;
    logPattern("OUT change");
  }

  if (!g245Enabled) {
    if (outChanged) {
      if (out == kArmSequence[armIndex]) {
        ++armIndex;

        if (armIndex >= (sizeof(kArmSequence) / sizeof(kArmSequence[0]))) {
          g245Enabled = true;
          patternIndex = 0;
          writePattern(kPatterns[patternIndex]);
          enable245();
          digitalWrite(LED_BUILTIN, HIGH);
          lastPatternMs = now;
          logPattern("245 enabled");
        } else {
          logPattern("arm step");
        }
      } else if (out == kArmSequence[0]) {
        armIndex = 1;
        logPattern("arm restart");
      } else if (out == 0x00) {
        // Stay idle; keep partial arm only if the ROM resumes quickly.
      } else {
        armIndex = 0;
      }
    }
  }

  if (g245Enabled && (uint32_t)(now - lastPatternMs) >= PATTERN_HOLD_MS) {
    lastPatternMs = now;
    patternIndex = (uint8_t)((patternIndex + 1U) % (sizeof(kPatterns) / sizeof(kPatterns[0])));
    writePattern(kPatterns[patternIndex]);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    logPattern("drive change");
  }

  if ((uint32_t)(now - lastHeartbeatMs) >= HEARTBEAT_MS) {
    lastHeartbeatMs = now;
    logPattern("heartbeat");
  }
}
