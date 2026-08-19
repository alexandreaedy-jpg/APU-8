#include <Arduino.h>
#include <SPI.h>

static const uint8_t PIN_MCP_MISO = 16;
static const uint8_t PIN_MCP_CS = 17;
static const uint8_t PIN_MCP_SCK = 18;
static const uint8_t PIN_MCP_MOSI = 19;

static const uint8_t PIN_PANEL_LFO_PITCH = 8;
static const uint8_t PIN_PANEL_LFO_DUTY = 9;
static const uint8_t PIN_PANEL_LFO_AMP = 15;
static const uint8_t PIN_PANEL_VOICE_P1 = 20;
static const uint8_t PIN_PANEL_VOICE_P2 = 21;
static const uint8_t PIN_PANEL_VOICE_TRI = 22;
static const uint8_t PIN_PANEL_VOICE_NOISE = 26;
static const uint8_t PIN_PANEL_VOICE_GLOBAL = 27;
static const uint8_t PIN_PANEL_ARP_ENABLE = 28;

static const bool PANEL_SWITCH_ACTIVE_LOW = true;
static const uint8_t MCP_CHANNELS = 8;
static const uint8_t SWITCH_COUNT = 9;
static const uint32_t PRINT_INTERVAL_MS = 100;

static const uint8_t kSwitchPins[SWITCH_COUNT] = {
  PIN_PANEL_LFO_PITCH,
  PIN_PANEL_LFO_DUTY,
  PIN_PANEL_LFO_AMP,
  PIN_PANEL_VOICE_P1,
  PIN_PANEL_VOICE_P2,
  PIN_PANEL_VOICE_TRI,
  PIN_PANEL_VOICE_NOISE,
  PIN_PANEL_VOICE_GLOBAL,
  PIN_PANEL_ARP_ENABLE
};

static const char* kSwitchLabels[SWITCH_COUNT] = {
  "LFO_PITCH",
  "LFO_DUTY",
  "LFO_AMP",
  "VOICE_P1",
  "VOICE_P2",
  "VOICE_TRI",
  "VOICE_NOISE",
  "VOICE_GLOBAL",
  "ARP"
};

static const char* kAnalogLabels[MCP_CHANNELS] = {
  "ATTACK",
  "VOLUME",
  "DECAY",
  "RELEASE",
  "LFO_DEPTH",
  "LFO_RATE",
  "DUTY",
  "ARP_TIME"
};

static uint16_t rawMin[MCP_CHANNELS];
static uint16_t rawMax[MCP_CHANNELS];
static uint16_t rawLast[MCP_CHANNELS];
static bool primed = false;
static uint32_t lastPrintMs = 0;

static inline bool panelPinActive(uint8_t pin) {
  const bool isHigh = (digitalRead(pin) == HIGH);
  return PANEL_SWITCH_ACTIVE_LOW ? !isHigh : isHigh;
}

static uint16_t readMcp3008(uint8_t channel) {
  channel &= 0x07U;
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_MCP_CS, LOW);
  SPI.transfer(0x01U);
  const uint8_t hi = SPI.transfer((uint8_t)(0x80U | (channel << 4)));
  const uint8_t lo = SPI.transfer(0x00U);
  digitalWrite(PIN_MCP_CS, HIGH);
  SPI.endTransaction();
  return (uint16_t)(((hi & 0x03U) << 8) | lo);
}

static void printAnalogBlock(const char* prefix, const uint16_t* values) {
  Serial.print(prefix);
  Serial.print('=');
  for (uint8_t i = 0; i < MCP_CHANNELS; ++i) {
    if (i != 0U) {
      Serial.print(',');
    }
    Serial.print(kAnalogLabels[i]);
    Serial.print(':');
    Serial.print(values[i]);
  }
}

static void printSwitchBlock() {
  Serial.print(F("SW="));
  for (uint8_t i = 0; i < SWITCH_COUNT; ++i) {
    if (i != 0U) {
      Serial.print(',');
    }
    Serial.print(kSwitchLabels[i]);
    Serial.print(':');
    Serial.print(panelPinActive(kSwitchPins[i]) ? F("ON") : F("off"));
  }
}

void setup() {
  Serial.begin(115200);
  delay(800);

  pinMode(PIN_MCP_CS, OUTPUT);
  digitalWrite(PIN_MCP_CS, HIGH);
  SPI.setRX(PIN_MCP_MISO);
  SPI.setTX(PIN_MCP_MOSI);
  SPI.setSCK(PIN_MCP_SCK);
  SPI.begin();

  for (uint8_t i = 0; i < SWITCH_COUNT; ++i) {
    pinMode(kSwitchPins[i], INPUT_PULLUP);
  }

  for (uint8_t i = 0; i < MCP_CHANNELS; ++i) {
    rawMin[i] = 1023U;
    rawMax[i] = 0U;
    rawLast[i] = 0U;
  }

  Serial.println(F("PicoPanelCtrlFullDiag start"));
  Serial.println(F("MCP3008 SPI: MISO=GP16 CS=GP17 SCK=GP18 MOSI=GP19"));
  Serial.println(F("Switches use INPUT_PULLUP: pressed/selected = ON when line is pulled to GND."));
  Serial.println(F("Expected panel groups:"));
  Serial.println(F("- LFO target: LFO_PITCH / LFO_DUTY / LFO_AMP"));
  Serial.println(F("- Voice select: VOICE_P1 / VOICE_P2 / VOICE_TRI / VOICE_NOISE / VOICE_GLOBAL"));
  Serial.println(F("- ARP button: ARP"));
}

void loop() {
  uint16_t raw[MCP_CHANNELS];
  uint8_t moveChannel = 0U;
  uint16_t moveDelta = 0U;
  const uint32_t nowMs = millis();

  for (uint8_t channel = 0; channel < MCP_CHANNELS; ++channel) {
    raw[channel] = readMcp3008(channel);
    if (raw[channel] < rawMin[channel]) rawMin[channel] = raw[channel];
    if (raw[channel] > rawMax[channel]) rawMax[channel] = raw[channel];

    const uint16_t delta = (raw[channel] > rawLast[channel])
      ? (uint16_t)(raw[channel] - rawLast[channel])
      : (uint16_t)(rawLast[channel] - raw[channel]);
    if (primed && delta > moveDelta) {
      moveDelta = delta;
      moveChannel = channel;
    }
    rawLast[channel] = raw[channel];
  }
  primed = true;

  if ((uint32_t)(nowMs - lastPrintMs) < PRINT_INTERVAL_MS) {
    return;
  }
  lastPrintMs = nowMs;

  Serial.print('[');
  Serial.print(nowMs);
  Serial.print(F(" ms] "));
  printAnalogBlock("RAW", raw);
  Serial.print(F(" | "));
  printAnalogBlock("MIN", rawMin);
  Serial.print(F(" | "));
  printAnalogBlock("MAX", rawMax);
  Serial.print(F(" | "));
  printSwitchBlock();
  Serial.print(F(" | MOVE="));
  Serial.print(kAnalogLabels[moveChannel]);
  Serial.print(':');
  Serial.println(moveDelta);
}
