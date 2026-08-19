#include <Arduino.h>

// V2_A / Step 1 OUT-only probe
//
// Goal:
// - keep the 74HCT245 disabled by default
// - observe only OUT0/OUT1/OUT2 coming from the NES through the 74HC4050
// - ignore /OE2 and A15 completely to remove noise from the logs

static const uint8_t PIN_OUT0 = 2;       // 74HC4050 output from NES OUT0
static const uint8_t PIN_OUT1 = 3;       // 74HC4050 output from NES OUT1
static const uint8_t PIN_OUT2 = 4;       // 74HC4050 output from NES OUT2
static const uint8_t PIN_245_OE = 7;     // 74HCT245 /OE, active low

static const uint8_t PIN_JOYPAD_D0 = 10; // reserved for later
static const uint8_t PIN_JOYPAD_D1 = 11; // reserved for later
static const uint8_t PIN_JOYPAD_D2 = 12; // reserved for later
static const uint8_t PIN_JOYPAD_D3 = 13; // reserved for later
static const uint8_t PIN_JOYPAD_D4 = 14; // reserved for later

static uint8_t lastOut = 0xFF;
static uint32_t lastHeartbeatMs = 0;

static inline uint8_t readOutBits() {
  return (uint8_t)((digitalRead(PIN_OUT0) ? 1 : 0) |
                   (digitalRead(PIN_OUT1) ? 2 : 0) |
                   (digitalRead(PIN_OUT2) ? 4 : 0));
}

static void disable245Safe() {
  pinMode(PIN_245_OE, OUTPUT);
  digitalWrite(PIN_245_OE, HIGH); // HIGH = disabled (tri-state)
}

static void setupJoypadOutputsIdle() {
  const uint8_t pins[5] = {
    PIN_JOYPAD_D0, PIN_JOYPAD_D1, PIN_JOYPAD_D2, PIN_JOYPAD_D3, PIN_JOYPAD_D4
  };

  for (uint8_t i = 0; i < 5; ++i) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

static void setupInputs() {
  pinMode(PIN_OUT0, INPUT);
  pinMode(PIN_OUT1, INPUT);
  pinMode(PIN_OUT2, INPUT);
}

static void logOutState(const char* reason) {
  const uint8_t out = readOutBits();

  Serial.print('[');
  Serial.print(millis());
  Serial.print(F(" ms] "));
  Serial.print(reason);
  Serial.print(F(" | OUT="));
  Serial.print((out >> 2) & 1);
  Serial.print((out >> 1) & 1);
  Serial.println(out & 1);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  disable245Safe();
  setupJoypadOutputsIdle();
  setupInputs();

  Serial.begin(115200);
  delay(250);
  Serial.println();
  Serial.println(F("PicoNesV2A_Step1OutOnly start"));
  Serial.println(F("74HCT245 forced SAFE: /OE held HIGH"));
  Serial.println(F("Watching only OUT0 OUT1 OUT2"));

  lastOut = readOutBits();
  logOutState("boot");
}

void loop() {
  const uint8_t out = readOutBits();

  if (out != lastOut) {
    lastOut = out;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    logOutState("OUT change");
  }

  const uint32_t now = millis();
  if ((uint32_t)(now - lastHeartbeatMs) >= 1000U) {
    lastHeartbeatMs = now;
    logOutState("heartbeat");
  }
}
