#include <Arduino.h>
#include <SPI.h>

static const uint8_t PIN_MCP_MISO = 16;
static const uint8_t PIN_MCP_CS = 17;
static const uint8_t PIN_MCP_SCK = 18;
static const uint8_t PIN_MCP_MOSI = 19;
static const uint8_t MCP_CHANNELS = 8;
static const uint32_t PRINT_INTERVAL_MS = 100;

static uint16_t rawMin[MCP_CHANNELS];
static uint16_t rawMax[MCP_CHANNELS];
static uint16_t rawLast[MCP_CHANNELS];
static bool primed = false;
static uint32_t lastPrintMs = 0;

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

static void printValues(const char* label, const uint16_t* values) {
  Serial.print(label);
  Serial.print('=');
  for (uint8_t channel = 0; channel < MCP_CHANNELS; ++channel) {
    if (channel != 0U) {
      Serial.print(',');
    }
    Serial.print(values[channel]);
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

  for (uint8_t channel = 0; channel < MCP_CHANNELS; ++channel) {
    rawMin[channel] = 1023U;
    rawMax[channel] = 0U;
    rawLast[channel] = 0U;
  }

  Serial.println(F("PicoMcp3008RawDiag start | DOUT=GP16 CS=GP17 CLK=GP18 DIN=GP19"));
  Serial.println(F("Test CH0: move Slider1, then swap to CH1, then force CH0 to GND and 3.3V."));
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
  printValues("RAW", raw);
  Serial.print(F(" | "));
  printValues("MIN", rawMin);
  Serial.print(F(" | "));
  printValues("MAX", rawMax);
  Serial.print(F(" | MOVE=CH"));
  Serial.print(moveChannel);
  Serial.print(':');
  Serial.println(moveDelta);
}
