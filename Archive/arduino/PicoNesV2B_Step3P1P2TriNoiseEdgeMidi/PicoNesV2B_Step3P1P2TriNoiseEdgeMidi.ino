#include <Arduino.h>

// V2_B / MIDI In P1/P2/TRI/NOISE
// Full-lane musical control-state map for:
// - P1 / P2 / TRI / NOISE / DMC samples
// - global duty + ADSR
// Goal: drive P1/P2/TRI directly from DIN MIDI on GP1, using the V18 channel map.

static const uint8_t PIN_MIDI_RX = 1;
static const uint8_t PIN_OUT0 = 2;
static const uint8_t PIN_OUT1 = 3;
static const uint8_t PIN_OUT2 = 4;
static const uint8_t PIN_OE2 = 5;
static const uint8_t PIN_A15 = 6;

static const uint8_t PIN_245_OE = 7;
static const uint8_t PIN_CPU_D0 = 0xFF;
static const uint8_t PIN_CPU_D1 = 0xFF;
static const uint8_t PIN_CPU_D2 = 0xFF;

static const uint8_t PIN_JOYPAD_D0 = 10;
static const uint8_t PIN_JOYPAD_D1 = 11;
static const uint8_t PIN_JOYPAD_D2 = 12;
static const uint8_t PIN_JOYPAD_D3 = 13;
static const uint8_t PIN_JOYPAD_D4 = 14;

static const uint8_t OPCODE_IDLE = 0x00;
static const uint8_t OPCODE_READ_STATUS = 0x01;
static const uint8_t OPCODE_READ_P1_LO = 0x02;
static const uint8_t OPCODE_READ_P1_HI = 0x03;
static const uint8_t OPCODE_READ_P2_LO = 0x04;
static const uint8_t OPCODE_READ_P2_HI = 0x05;
static const uint8_t OPCODE_READ_AUX_LO = 0x06;
static const uint8_t OPCODE_READ_AUX_HI = 0x07;

static const uint8_t STATUS_BIT_P1_PENDING = 0x01;
static const uint8_t STATUS_BIT_P2_PENDING = 0x02;
static const uint8_t STATUS_BIT_AUX_PENDING = 0x04;
static const uint8_t STATUS_BIT_AUX_IS_TRI = 0x08;
static const uint8_t STATUS_VALID_ONLY = 0x10;
static const uint8_t STATUS_SIGNATURE = STATUS_VALID_ONLY;

static const uint32_t ARM_OUT_SETTLE_US = 200U;
static const uint32_t ACTIVE_OUT_SETTLE_US = 2U;
static const uint32_t MIDI_BAUD = 31250U;
static const uint32_t HEARTBEAT_IDLE_MS = 4000U;
static const uint32_t HEARTBEAT_ACTIVE_MS = 1500U;
static const uint32_t HEARTBEAT_BURST_MS = 120U;
static const uint32_t HEARTBEAT_RECENT_MIDI_MS = 2500U;
static const uint8_t HEARTBEAT_BURST_COUNT = 10U;
static const uint32_t ARM_FALLBACK_MS = 1500U;
static const bool ENABLE_ARM_FALLBACK = true;
static const bool ENABLE_RELOAD_DETECTOR = false;
static const bool VERBOSE_BUS_LOGS = false;
static const bool ENABLE_BUS_SNIFF = false;
static const bool ENABLE_HEARTBEAT_BURST = false;
static const char FW_REV[] = "t25k-v2b-perf";

static const uint8_t kArmSequence[] = {0x04, 0x02, 0x01, 0x07};

struct ProtoEvent {
  uint8_t regId;
  uint16_t value;
};

struct VoiceEdgeQueue {
  uint8_t values[16];
  uint8_t head;
  uint8_t tail;
  uint8_t count;
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

struct MidiVoice {
  bool active;
  uint8_t midiNote;
  uint8_t nesNote;
  uint8_t stackCount;
  uint8_t stack[4];
};

struct BusSniffSnapshot {
  uint32_t oe2Falls;
  uint32_t oe2RaceCount;
  uint32_t oe2UnexpectedOutCount;
  uint32_t oe2A15HighCount;
  uint32_t oe2ReadStatusCount;
  uint32_t oe2ReadP1Count;
  uint32_t oe2ReadP2Count;
  uint32_t oe2ReadAuxCount;
  uint32_t oe2ReadStatusA15HighCount;
  uint32_t oe2ReadP1A15HighCount;
  uint32_t oe2ReadP2A15HighCount;
  uint32_t oe2ReadAuxA15HighCount;
  uint32_t oe2LowWidthMinUs;
  uint32_t oe2LowWidthMaxUs;
  uint32_t oe2IntervalMinUs;
  uint32_t oe2IntervalMaxUs;
  uint32_t outToOe2MinUs;
  uint32_t outToOe2MaxUs;
  uint8_t cpuSniffConfigured;
};

// V18 MIDI map. Keep these fixed so the Pico path matches the known-good Nano/V18 workflow.
static const uint8_t CH_P1 = 12;
static const uint8_t CH_P2 = 13;
static const uint8_t CH_TRI = 14;
static const uint8_t CH_NOISE = 15;
static const uint8_t CH_GLOBAL = 16;

static const uint8_t VOICE_P1 = 0;
static const uint8_t VOICE_P2 = 1;
static const uint8_t VOICE_TRI = 2;
static const uint8_t VOICE_NOISE = 3;
static const uint8_t VOICE_NONE = 0xFF;
static const uint8_t VOICE_COUNT = 4;

static const uint8_t VOICE_CODE_P1 = 0;
static const uint8_t VOICE_CODE_P2 = 1;
static const uint8_t VOICE_CODE_TRI = 2;
static const uint8_t VOICE_CODE_NOISE = 3;

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
static const uint8_t REG_PULSE_FRAME = REG_P1_TRIG;
static const uint8_t REG_P2_NOTE = 0xB;
static const uint8_t REG_P2_GATE = 0xC;
static const uint8_t REG_P2_TRIG = 0xD;
static const uint8_t REG_NOI_NOTE = 0xE;
static const uint8_t REG_NOI_GATE = 0xF;
static const uint8_t COMPACT_GATE_BIT = 0x80;

static inline uint8_t packDutyLfo(uint8_t duty, uint8_t depth, uint8_t rate) {
  return (uint8_t)((duty & 0x03U) | ((depth & 0x07U) << 2) | ((rate & 0x07U) << 5));
}

static const ControlState kInitialState = {
  40, 52, 40,
  0, 0, 0,
  0, 0, 0,
  0, 0, 15, 0,
  2, 0, 0,
  24, 0
};

static const uint8_t QUEUE_CAPACITY = 32;
static ProtoEvent queueBuf[QUEUE_CAPACITY];
static uint8_t queueHead = 0;
static uint8_t queueTail = 0;
static uint8_t queueCount = 0;
static VoiceEdgeQueue voiceQueues[VOICE_COUNT];
static bool preAckVoiceValid[VOICE_COUNT] = {false, false, false, false};
static uint8_t preAckVoiceValue[VOICE_COUNT] = {0, 0, 0, 0};
static ProtoEvent frontVoiceEvent = {REG_P1_NOTE, 0};
static uint8_t slotScanStart = 0;
static bool transportPrimed = false;
static uint8_t warmupDirtyMask = 0;
static bool latchedBatchActive = false;
static bool latchedP1Pending = false;
static bool latchedP2Pending = false;
static uint8_t latchedP1Value = 0;
static uint8_t latchedP2Value = 0;
static bool latchedAuxPending = false;
static uint8_t latchedAuxVoice = VOICE_NONE;
static uint8_t latchedAuxValue = 0;

static bool g245Enabled = false;
static bool overflowLatched = false;
static uint8_t currentPattern = STATUS_VALID_ONLY;
static volatile uint8_t currentOut = 0xFF;
static volatile uint8_t committedOut = 0xFF;
static uint8_t armIndex = 0;
static uint8_t reloadIndex = 0;
static ControlState currentControlState = kInitialState;
static bool sawFirstAck = false;
static uint32_t lastHeartbeatMs = 0;
static uint32_t lastMidiEventMs = 0;
static bool lastAuxConflictActive = false;
static uint8_t heartbeatBurstRemaining = 0;
static uint32_t lastMidiPressureLogMs = 0;
static bool overflowPressureLogged = false;
static volatile uint32_t outCandidateSinceUs = 0;
static uint32_t bootMs = 0;
static uint32_t firstAckMs = 0;
static uint8_t midiRunningStatus = 0;
static uint8_t midiData[2] = {0, 0};
static uint8_t midiDataCount = 0;
static uint8_t midiExpectedData = 0;
static uint32_t midiFramesEnqueued = 0;
static uint32_t midiFramesAcked = 0;
static uint32_t midiFramesReplaced = 0;
static uint32_t midiIgnoredNoteOffs = 0;
static uint32_t midiResumedNotes = 0;
static uint32_t midiPreAckLatched = 0;
static uint32_t armStepCount = 0;
static uint32_t armRestartCount = 0;
static uint32_t armResetCount = 0;
static uint32_t armFallbackCount = 0;
static uint32_t reloadCount = 0;
static uint8_t queueHighWater = 0;
static volatile uint32_t lastCommittedOutChangeUs = 0;
static volatile bool lastOe2Level = true;
static volatile uint32_t oe2FallCount = 0;
static volatile uint32_t oe2RaceCount = 0;
static volatile uint32_t oe2UnexpectedOutCount = 0;
static volatile uint32_t oe2A15HighCount = 0;
static volatile uint32_t oe2LowWidthMinUs = 0xFFFFFFFFUL;
static volatile uint32_t oe2LowWidthMaxUs = 0;
static volatile uint32_t oe2LowWidthLastUs = 0;
static volatile uint32_t oe2IntervalMinUs = 0xFFFFFFFFUL;
static volatile uint32_t oe2IntervalMaxUs = 0;
static volatile uint32_t oe2IntervalLastUs = 0;
static volatile uint32_t outToOe2MinUs = 0xFFFFFFFFUL;
static volatile uint32_t outToOe2MaxUs = 0;
static volatile uint32_t outToOe2LastUs = 0;
static volatile uint32_t oe2LowStartUs = 0;
static volatile uint32_t lastOe2FallUs = 0;
static volatile uint32_t oe2ByOut[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static volatile uint32_t oe2A15HighByOut[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static volatile uint32_t cpuBitsByOut[8][8] = {};
static MidiVoice midiVoices[VOICE_COUNT] = {
  {false, 0, 40, 0, {0, 0, 0, 0}},
  {false, 0, 52, 0, {0, 0, 0, 0}},
  {false, 0, 40, 0, {0, 0, 0, 0}},
  {false, 0, 0, 0, {0, 0, 0, 0}}
};

static inline uint8_t readOutBits() {
  return (uint8_t)((digitalRead(PIN_OUT0) ? 1 : 0) |
                   (digitalRead(PIN_OUT1) ? 2 : 0) |
                   (digitalRead(PIN_OUT2) ? 4 : 0));
}

static inline bool pinConfigured(uint8_t pin) {
  return pin != 0xFF;
}

static inline bool cpuSniffConfigured() {
  return pinConfigured(PIN_CPU_D0) &&
         pinConfigured(PIN_CPU_D1) &&
         pinConfigured(PIN_CPU_D2);
}

static inline uint8_t readCpuSniffBits() {
  if (!cpuSniffConfigured()) return 0xFF;
  return (uint8_t)((digitalRead(PIN_CPU_D0) ? 1 : 0) |
                   (digitalRead(PIN_CPU_D1) ? 2 : 0) |
                   (digitalRead(PIN_CPU_D2) ? 4 : 0));
}

static inline bool outIsReadOpcode(uint8_t out) {
  switch (out & 0x07U) {
    case OPCODE_READ_STATUS:
    case OPCODE_READ_P1_LO:
    case OPCODE_READ_P1_HI:
    case OPCODE_READ_P2_LO:
    case OPCODE_READ_P2_HI:
    case OPCODE_READ_AUX_LO:
    case OPCODE_READ_AUX_HI:
      return true;
    default:
      return false;
  }
}

static void updateSniffRange(volatile uint32_t& minValue,
                             volatile uint32_t& maxValue,
                             volatile uint32_t& lastValue,
                             uint32_t sample) {
  lastValue = sample;
  if (sample < minValue) minValue = sample;
  if (sample > maxValue) maxValue = sample;
}

static void resetBusSniffStats() {
  noInterrupts();
  oe2FallCount = 0;
  oe2RaceCount = 0;
  oe2UnexpectedOutCount = 0;
  oe2A15HighCount = 0;
  oe2LowWidthMinUs = 0xFFFFFFFFUL;
  oe2LowWidthMaxUs = 0;
  oe2LowWidthLastUs = 0;
  oe2IntervalMinUs = 0xFFFFFFFFUL;
  oe2IntervalMaxUs = 0;
  oe2IntervalLastUs = 0;
  outToOe2MinUs = 0xFFFFFFFFUL;
  outToOe2MaxUs = 0;
  outToOe2LastUs = 0;
  oe2LowStartUs = 0;
  lastOe2FallUs = 0;
  for (uint8_t i = 0; i < 8; ++i) {
    oe2ByOut[i] = 0;
    oe2A15HighByOut[i] = 0;
    for (uint8_t j = 0; j < 8; ++j) {
      cpuBitsByOut[i][j] = 0;
    }
  }
  interrupts();
}

static void oe2EdgeIsr() {
  if (!ENABLE_BUS_SNIFF) return;

  const bool level = digitalRead(PIN_OE2);
  const uint32_t nowUs = micros();

  if (!level) {
    const uint8_t committed = committedOut;
    const uint8_t sampled = currentOut;
    const uint32_t deltaUs = nowUs - lastCommittedOutChangeUs;
    const bool a15High = pinConfigured(PIN_A15) && digitalRead(PIN_A15);
    ++oe2FallCount;
    oe2LowStartUs = nowUs;
    if (lastOe2FallUs != 0) {
      updateSniffRange(oe2IntervalMinUs, oe2IntervalMaxUs, oe2IntervalLastUs, nowUs - lastOe2FallUs);
    }
    lastOe2FallUs = nowUs;
    updateSniffRange(outToOe2MinUs, outToOe2MaxUs, outToOe2LastUs, deltaUs);
    if (a15High) {
      ++oe2A15HighCount;
    }
    if (sampled != committed) {
      ++oe2RaceCount;
    }
    if (!outIsReadOpcode(committed)) {
      ++oe2UnexpectedOutCount;
    }
    ++oe2ByOut[committed & 0x07U];
    if (a15High) {
      ++oe2A15HighByOut[committed & 0x07U];
    }
    if (cpuSniffConfigured()) {
      const uint8_t cpuBits = readCpuSniffBits();
      if (cpuBits < 8U) {
        ++cpuBitsByOut[committed & 0x07U][cpuBits];
      }
    }
  } else if (!lastOe2Level && oe2LowStartUs != 0) {
    updateSniffRange(oe2LowWidthMinUs, oe2LowWidthMaxUs, oe2LowWidthLastUs, nowUs - oe2LowStartUs);
  }

  lastOe2Level = level;
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
  if (pinConfigured(PIN_OE2)) pinMode(PIN_OE2, INPUT);
  if (pinConfigured(PIN_A15)) pinMode(PIN_A15, INPUT);
  if (pinConfigured(PIN_CPU_D0)) pinMode(PIN_CPU_D0, INPUT);
  if (pinConfigured(PIN_CPU_D1)) pinMode(PIN_CPU_D1, INPUT);
  if (pinConfigured(PIN_CPU_D2)) pinMode(PIN_CPU_D2, INPUT);
}

static void writePattern(uint8_t pattern) {
  digitalWrite(PIN_JOYPAD_D0, (pattern & 0x01U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D1, (pattern & 0x02U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D2, (pattern & 0x04U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D3, (pattern & 0x08U) ? LOW : HIGH);
  digitalWrite(PIN_JOYPAD_D4, (pattern & 0x10U) ? LOW : HIGH);
  currentPattern = pattern;
}

static uint8_t voiceCodeForIndex(uint8_t voiceIndex) {
  switch (voiceIndex) {
    case VOICE_P1: return VOICE_CODE_P1;
    case VOICE_P2: return VOICE_CODE_P2;
    case VOICE_TRI: return VOICE_CODE_TRI;
    case VOICE_NOISE: return VOICE_CODE_NOISE;
    default: return VOICE_CODE_P1;
  }
}

static const __FlashStringHelper* voiceNameForIndex(uint8_t voiceIndex) {
  switch (voiceIndex) {
    case VOICE_P1: return F("P1");
    case VOICE_P2: return F("P2");
    case VOICE_TRI: return F("TRI");
    case VOICE_NOISE: return F("NOI");
    default: return F("--");
  }
}

static inline uint8_t pendingCountForVoice(uint8_t voiceIndex) {
  return (uint8_t)(voiceQueues[voiceIndex].count + (preAckVoiceValid[voiceIndex] ? 1U : 0U));
}

static inline uint8_t totalPendingCount() {
  uint8_t total = 0;
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    total = (uint8_t)(total + pendingCountForVoice(voiceIndex));
  }
  return total;
}

static inline bool voiceHasPending(uint8_t voiceIndex) {
  return pendingCountForVoice(voiceIndex) != 0U;
}

static inline uint8_t frontVoiceValue(uint8_t voiceIndex) {
  const VoiceEdgeQueue& queue = voiceQueues[voiceIndex];
  if (queue.count != 0U) {
    return queue.values[queue.head];
  }
  return preAckVoiceValue[voiceIndex];
}

static inline bool compactValueGateOn(uint8_t value) {
  return (value & COMPACT_GATE_BIT) != 0U;
}

static uint8_t auxVoicePriorityScore(uint8_t voiceIndex) {
  uint8_t score = 0;
  uint8_t pending = pendingCountForVoice(voiceIndex);
  const uint8_t value = frontVoiceValue(voiceIndex);

  if (compactValueGateOn(value)) {
    score = (uint8_t)(score + 8U);
    if (voiceIndex == VOICE_NOISE) {
      score = (uint8_t)(score + 2U);
    }
  }

  if (pending > 3U) {
    pending = 3U;
  }
  score = (uint8_t)(score + pending);

  if (voiceIndex != slotScanStart) {
    score = (uint8_t)(score + 1U);
  }
  return score;
}

static inline bool auxVoiceSelectable(uint8_t voiceIndex) {
  return voiceIndex == VOICE_TRI || voiceIndex == VOICE_NOISE;
}

static inline bool auxVoicesContending() {
  return voiceHasPending(VOICE_TRI) && voiceHasPending(VOICE_NOISE);
}

static inline bool midiActivityRecent(uint32_t nowMs) {
  return lastMidiEventMs != 0U && (uint32_t)(nowMs - lastMidiEventMs) <= HEARTBEAT_RECENT_MIDI_MS;
}

static void requestHeartbeatBurst(uint8_t count) {
  if (!ENABLE_HEARTBEAT_BURST) {
    return;
  }
  if (count > heartbeatBurstRemaining) {
    heartbeatBurstRemaining = count;
  }
}

static uint32_t currentHeartbeatIntervalMs(uint32_t nowMs) {
  if (!sawFirstAck || !transportPrimed) {
    return HEARTBEAT_IDLE_MS;
  }
  if (heartbeatBurstRemaining != 0U) {
    return HEARTBEAT_BURST_MS;
  }
  if (midiActivityRecent(nowMs)) {
    return HEARTBEAT_ACTIVE_MS;
  }
  return HEARTBEAT_IDLE_MS;
}

static inline uint8_t voiceMaskBit(uint8_t voiceIndex) {
  return (uint8_t)(1U << voiceIndex);
}

static void selectAuxSnapshot(uint8_t* auxVoice, uint8_t* auxValue) {
  const bool triPending = voiceHasPending(VOICE_TRI);
  const bool noisePending = voiceHasPending(VOICE_NOISE);

  *auxVoice = VOICE_NONE;
  *auxValue = 0;

  if (!triPending && !noisePending) {
    return;
  }

  if (triPending && !noisePending) {
    *auxVoice = VOICE_TRI;
    *auxValue = frontVoiceValue(VOICE_TRI);
    return;
  }

  if (noisePending && !triPending) {
    *auxVoice = VOICE_NOISE;
    *auxValue = frontVoiceValue(VOICE_NOISE);
    return;
  }

  const uint8_t triValue = frontVoiceValue(VOICE_TRI);
  const uint8_t noiseValue = frontVoiceValue(VOICE_NOISE);
  const bool triGateOn = compactValueGateOn(triValue);
  const bool noiseGateOn = compactValueGateOn(noiseValue);

  if (triGateOn != noiseGateOn) {
    if (noiseGateOn) {
      *auxVoice = VOICE_NOISE;
      *auxValue = noiseValue;
    } else {
      *auxVoice = VOICE_TRI;
      *auxValue = triValue;
    }
    return;
  }

  const uint8_t triScore = auxVoicePriorityScore(VOICE_TRI);
  const uint8_t noiseScore = auxVoicePriorityScore(VOICE_NOISE);

  if (noiseScore > triScore) {
    *auxVoice = VOICE_NOISE;
    *auxValue = noiseValue;
    return;
  }

  if (triScore > noiseScore) {
    *auxVoice = VOICE_TRI;
    *auxValue = triValue;
    return;
  }

  if (slotScanStart == VOICE_NOISE) {
    *auxVoice = VOICE_NOISE;
    *auxValue = noiseValue;
  } else {
    *auxVoice = VOICE_TRI;
    *auxValue = triValue;
  }
}

static void latchTransportSnapshot() {
  latchedP1Pending = voiceHasPending(VOICE_P1);
  latchedP2Pending = voiceHasPending(VOICE_P2);
  latchedP1Value = latchedP1Pending ? frontVoiceValue(VOICE_P1) : 0;
  latchedP2Value = latchedP2Pending ? frontVoiceValue(VOICE_P2) : 0;
  selectAuxSnapshot(&latchedAuxVoice, &latchedAuxValue);
  latchedAuxPending = (latchedAuxVoice != VOICE_NONE);
  latchedBatchActive = true;
}

static void clearLatchedBatch() {
  latchedBatchActive = false;
  latchedP1Pending = false;
  latchedP2Pending = false;
  latchedP1Value = 0;
  latchedP2Value = 0;
  latchedAuxPending = false;
  latchedAuxVoice = VOICE_NONE;
  latchedAuxValue = 0;
}

static inline void maybePrimeTransport() {
  if (!transportPrimed && sawFirstAck && warmupDirtyMask == 0U) {
    transportPrimed = true;
  }
}

static void touchQueueHighWater() {
  const uint8_t totalPending = totalPendingCount();
  if (totalPending > queueHighWater) {
    queueHighWater = totalPending;
  }
}

static const ProtoEvent* frontEvent() {
  if (voiceHasPending(VOICE_P1)) {
    frontVoiceEvent.regId = REG_P1_NOTE;
    frontVoiceEvent.value = frontVoiceValue(VOICE_P1);
    return &frontVoiceEvent;
  }
  if (voiceHasPending(VOICE_P2)) {
    frontVoiceEvent.regId = REG_P2_NOTE;
    frontVoiceEvent.value = frontVoiceValue(VOICE_P2);
    return &frontVoiceEvent;
  }
  if (voiceHasPending(VOICE_TRI)) {
    frontVoiceEvent.regId = REG_TRI_NOTE;
    frontVoiceEvent.value = frontVoiceValue(VOICE_TRI);
    return &frontVoiceEvent;
  }
  if (voiceHasPending(VOICE_NOISE)) {
    frontVoiceEvent.regId = REG_NOI_NOTE;
    frontVoiceEvent.value = frontVoiceValue(VOICE_NOISE);
    return &frontVoiceEvent;
  }
  return nullptr;
}

static bool isMidiTransportEventReg(uint8_t regId) {
  return regId == REG_P1_NOTE || regId == REG_P2_NOTE || regId == REG_TRI_NOTE;
}

static void printQueueState() {
  const uint8_t p1Pending = pendingCountForVoice(VOICE_P1);
  const uint8_t p2Pending = pendingCountForVoice(VOICE_P2);
  const uint8_t triPending = pendingCountForVoice(VOICE_TRI);
  const uint8_t noisePending = pendingCountForVoice(VOICE_NOISE);

  Serial.print(F(" | Q="));
  Serial.print(totalPendingCount());
  Serial.print(F(" | QH="));
  Serial.print(queueHighWater);
  Serial.print(F(" | OVF="));
  Serial.print(overflowLatched ? 1 : 0);
  Serial.print(F(" | AX="));
  Serial.print(voiceNameForIndex(latchedAuxVoice));
  Serial.print(':');
  if (!latchedAuxPending) {
    Serial.print(F("--"));
  } else if (latchedAuxValue < 16U) {
    Serial.print('0');
    Serial.print(latchedAuxValue, HEX);
  } else {
    Serial.print(latchedAuxValue, HEX);
  }
  Serial.print(F(" | BX="));
  Serial.print(F("--"));
  Serial.print(F(" | P1="));
  if (!voiceHasPending(VOICE_P1)) {
    Serial.print(F("--"));
  } else {
    const uint8_t value = frontVoiceValue(VOICE_P1);
    if (value < 16U) {
      Serial.print('0');
    }
    Serial.print(value, HEX);
  }
  Serial.print('/');
  Serial.print(p1Pending);
  Serial.print(F(" | P2="));
  if (!voiceHasPending(VOICE_P2)) {
    Serial.print(F("--"));
  } else {
    const uint8_t value = frontVoiceValue(VOICE_P2);
    if (value < 16U) {
      Serial.print('0');
    }
    Serial.print(value, HEX);
  }
  Serial.print('/');
  Serial.print(p2Pending);
  Serial.print(F(" | T="));
  if (!voiceHasPending(VOICE_TRI)) {
    Serial.print(F("--"));
  } else {
    const uint8_t value = frontVoiceValue(VOICE_TRI);
    if (value < 16U) {
      Serial.print('0');
    }
    Serial.print(value, HEX);
  }
  Serial.print('/');
  Serial.print(triPending);
  Serial.print(F(" | N="));
  if (!voiceHasPending(VOICE_NOISE)) {
    Serial.print(F("--"));
  } else {
    const uint8_t value = frontVoiceValue(VOICE_NOISE);
    if (value < 16U) {
      Serial.print('0');
    }
    Serial.print(value, HEX);
  }
  Serial.print('/');
  Serial.print(noisePending);
}

static void printMidiState() {
  Serial.print(F(" | MF="));
  Serial.print(midiFramesEnqueued);
  Serial.print('/');
  Serial.print(midiFramesAcked);
  Serial.print(F(" | MX="));
  Serial.print(midiFramesReplaced);
  Serial.print(F(" | MI="));
  Serial.print(midiIgnoredNoteOffs);
  Serial.print(F(" | MR="));
  Serial.print(midiResumedNotes);
  Serial.print(F(" | MB="));
  Serial.print(midiPreAckLatched);
  Serial.print(F(" | TP="));
  Serial.print(transportPrimed ? F("LIVE") : F("WARM"));
  Serial.print(F(" | WM="));
  Serial.print(warmupDirtyMask, HEX);
  Serial.print(F(" | FW="));
  Serial.print(FW_REV);
}

static void printArmState() {
  Serial.print(F(" | ARM="));
  Serial.print(armStepCount);
  Serial.print('/');
  Serial.print(armRestartCount);
  Serial.print('/');
  Serial.print(armResetCount);
  Serial.print('/');
  Serial.print(armFallbackCount);
  Serial.print('/');
  Serial.print(reloadCount);
  Serial.print(F(" | ACK0="));
  if (firstAckMs == 0) {
    Serial.print(F("--"));
  } else {
    Serial.print(firstAckMs - bootMs);
  }
}

static void printSniffRange(uint32_t minValue, uint32_t maxValue) {
  if (minValue == 0xFFFFFFFFUL) {
    Serial.print(F("--"));
    return;
  }
  Serial.print(minValue);
  Serial.print('/');
  Serial.print(maxValue);
}

static void printBusSniffState() {
  BusSniffSnapshot snapshot;

  snapshot.cpuSniffConfigured = cpuSniffConfigured() ? 1U : 0U;
  if (!ENABLE_BUS_SNIFF) {
    Serial.print(F(" | OE2=off"));
    return;
  }

  noInterrupts();
  snapshot.oe2Falls = oe2FallCount;
  snapshot.oe2RaceCount = oe2RaceCount;
  snapshot.oe2UnexpectedOutCount = oe2UnexpectedOutCount;
  snapshot.oe2A15HighCount = oe2A15HighCount;
  snapshot.oe2ReadStatusCount = oe2ByOut[OPCODE_READ_STATUS];
  snapshot.oe2ReadP1Count = oe2ByOut[OPCODE_READ_P1_LO] + oe2ByOut[OPCODE_READ_P1_HI];
  snapshot.oe2ReadP2Count = oe2ByOut[OPCODE_READ_P2_LO] + oe2ByOut[OPCODE_READ_P2_HI];
  snapshot.oe2ReadAuxCount = oe2ByOut[OPCODE_READ_AUX_LO] + oe2ByOut[OPCODE_READ_AUX_HI];
  snapshot.oe2ReadStatusA15HighCount = oe2A15HighByOut[OPCODE_READ_STATUS];
  snapshot.oe2ReadP1A15HighCount = oe2A15HighByOut[OPCODE_READ_P1_LO] + oe2A15HighByOut[OPCODE_READ_P1_HI];
  snapshot.oe2ReadP2A15HighCount = oe2A15HighByOut[OPCODE_READ_P2_LO] + oe2A15HighByOut[OPCODE_READ_P2_HI];
  snapshot.oe2ReadAuxA15HighCount = oe2A15HighByOut[OPCODE_READ_AUX_LO] + oe2A15HighByOut[OPCODE_READ_AUX_HI];
  snapshot.oe2LowWidthMinUs = oe2LowWidthMinUs;
  snapshot.oe2LowWidthMaxUs = oe2LowWidthMaxUs;
  snapshot.oe2IntervalMinUs = oe2IntervalMinUs;
  snapshot.oe2IntervalMaxUs = oe2IntervalMaxUs;
  snapshot.outToOe2MinUs = outToOe2MinUs;
  snapshot.outToOe2MaxUs = outToOe2MaxUs;
  interrupts();

  Serial.print(F(" | OE2="));
  Serial.print(snapshot.oe2Falls);
  Serial.print(F(" | RACE="));
  Serial.print(snapshot.oe2RaceCount);
  Serial.print(F(" | UNX="));
  Serial.print(snapshot.oe2UnexpectedOutCount);
  Serial.print(F(" | A15R="));
  Serial.print(snapshot.oe2A15HighCount);
  Serial.print(F(" | RO="));
  Serial.print(snapshot.oe2ReadStatusCount);
  Serial.print(',');
  Serial.print(snapshot.oe2ReadP1Count);
  Serial.print(',');
  Serial.print(snapshot.oe2ReadP2Count);
  Serial.print(',');
  Serial.print(snapshot.oe2ReadAuxCount);
  Serial.print(F(" | A15O="));
  Serial.print(snapshot.oe2ReadStatusA15HighCount);
  Serial.print(',');
  Serial.print(snapshot.oe2ReadP1A15HighCount);
  Serial.print(',');
  Serial.print(snapshot.oe2ReadP2A15HighCount);
  Serial.print(',');
  Serial.print(snapshot.oe2ReadAuxA15HighCount);
  Serial.print(F(" | O2E="));
  printSniffRange(snapshot.outToOe2MinUs, snapshot.outToOe2MaxUs);
  Serial.print(F(" | OI="));
  printSniffRange(snapshot.oe2IntervalMinUs, snapshot.oe2IntervalMaxUs);
  Serial.print(F(" | OW="));
  printSniffRange(snapshot.oe2LowWidthMinUs, snapshot.oe2LowWidthMaxUs);
  Serial.print(F(" | CPU="));
  Serial.print(snapshot.cpuSniffConfigured ? F("ON") : F("OFF"));
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
  printMidiState();
  printArmState();
  printBusSniffState();
  Serial.println();
}

static uint8_t buildStatusPattern() {
  uint8_t pattern = STATUS_SIGNATURE;
  if (latchedP1Pending) pattern |= STATUS_BIT_P1_PENDING;
  if (latchedP2Pending) pattern |= STATUS_BIT_P2_PENDING;
  if (latchedAuxPending) pattern |= STATUS_BIT_AUX_PENDING;
  if (latchedAuxPending && latchedAuxVoice == VOICE_TRI) pattern |= STATUS_BIT_AUX_IS_TRI;
  return pattern;
}

static uint8_t latchAndBuildStatusPattern() {
  latchTransportSnapshot();
  return buildStatusPattern();
}

static uint8_t buildP1LoPattern() {
  return latchedP1Pending ? (uint8_t)(STATUS_VALID_ONLY | (latchedP1Value & 0x0FU))
                          : STATUS_VALID_ONLY;
}

static uint8_t buildP1HiPattern() {
  return latchedP1Pending ? (uint8_t)(STATUS_VALID_ONLY | ((latchedP1Value >> 4) & 0x0FU))
                          : STATUS_VALID_ONLY;
}

static uint8_t buildP2LoPattern() {
  return latchedP2Pending ? (uint8_t)(STATUS_VALID_ONLY | (latchedP2Value & 0x0FU))
                          : STATUS_VALID_ONLY;
}

static uint8_t buildP2HiPattern() {
  return latchedP2Pending ? (uint8_t)(STATUS_VALID_ONLY | ((latchedP2Value >> 4) & 0x0FU))
                          : STATUS_VALID_ONLY;
}

static uint8_t buildAuxLoPattern() {
  if (!latchedAuxPending) return STATUS_VALID_ONLY;
  return (uint8_t)(STATUS_VALID_ONLY | (latchedAuxValue & 0x0FU));
}

static uint8_t buildAuxHiPattern() {
  if (!latchedAuxPending) return STATUS_VALID_ONLY;
  return (uint8_t)(STATUS_VALID_ONLY | ((latchedAuxValue >> 4) & 0x0FU));
}

static void clearQueue() {
  queueHead = 0;
  queueTail = 0;
  queueCount = 0;
  slotScanStart = VOICE_TRI;
  transportPrimed = false;
  warmupDirtyMask = 0;
  latchedBatchActive = false;
  latchedP1Pending = false;
  latchedP2Pending = false;
  latchedP1Value = 0;
  latchedP2Value = 0;
  latchedAuxPending = false;
  latchedAuxVoice = VOICE_NONE;
  latchedAuxValue = 0;
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    voiceQueues[voiceIndex].head = 0;
    voiceQueues[voiceIndex].tail = 0;
    voiceQueues[voiceIndex].count = 0;
    preAckVoiceValid[voiceIndex] = false;
    preAckVoiceValue[voiceIndex] = 0;
  }
  overflowLatched = false;
}

static bool enqueueVoiceEvent(uint8_t voiceIndex, uint8_t value) {
  VoiceEdgeQueue& queue = voiceQueues[voiceIndex];
  if (queue.count >= (uint8_t)(sizeof(queue.values) / sizeof(queue.values[0]))) {
    overflowLatched = true;
    return false;
  }
  queue.values[queue.tail] = value;
  queue.tail = (uint8_t)((queue.tail + 1U) % (sizeof(queue.values) / sizeof(queue.values[0])));
  ++queue.count;
  touchQueueHighWater();
  return true;
}

static void latchPreAckVoice(uint8_t voiceIndex, uint8_t value) {
  preAckVoiceValue[voiceIndex] = value;
  preAckVoiceValid[voiceIndex] = true;
  warmupDirtyMask |= voiceMaskBit(voiceIndex);
  touchQueueHighWater();
}

static bool ackVoiceEvent(uint8_t voiceIndex) {
  VoiceEdgeQueue& queue = voiceQueues[voiceIndex];

  if (queue.count != 0U) {
    queue.head = (uint8_t)((queue.head + 1U) % (sizeof(queue.values) / sizeof(queue.values[0])));
    --queue.count;
  } else if (preAckVoiceValid[voiceIndex]) {
    preAckVoiceValid[voiceIndex] = false;
    preAckVoiceValue[voiceIndex] = 0;
  } else {
    return false;
  }

  ++midiFramesAcked;
  if (!transportPrimed) {
    warmupDirtyMask &= (uint8_t)~voiceMaskBit(voiceIndex);
  }
  if (totalPendingCount() == 0U) {
    overflowLatched = false;
  }
  maybePrimeTransport();
  return true;
}

static bool ackBatchEvents() {
  bool acked = false;
  if (latchedP1Pending && ackVoiceEvent(VOICE_P1)) acked = true;
  if (latchedP2Pending && ackVoiceEvent(VOICE_P2)) acked = true;
  if (latchedAuxPending && ackVoiceEvent(latchedAuxVoice)) {
    acked = true;
    if (latchedAuxVoice == VOICE_TRI) {
      slotScanStart = VOICE_NOISE;
    } else if (latchedAuxVoice == VOICE_NOISE) {
      slotScanStart = VOICE_TRI;
    }
  }
  if (acked || latchedBatchActive) {
    clearLatchedBatch();
  }
  return acked;
}

static bool enqueueEvent(const ProtoEvent& evt) {
  if (queueCount >= QUEUE_CAPACITY) {
    overflowLatched = true;
    return false;
  }
  queueBuf[queueTail] = evt;
  queueTail = (uint8_t)((queueTail + 1U) % QUEUE_CAPACITY);
  ++queueCount;
  if (queueCount > queueHighWater) {
    queueHighWater = queueCount;
  }
  return true;
}

static uint8_t queueIndexAt(uint8_t logicalOffset) {
  return (uint8_t)((queueHead + logicalOffset) % QUEUE_CAPACITY);
}

static inline bool compactGate(uint8_t value) {
  return (value & COMPACT_GATE_BIT) != 0;
}

static inline uint8_t pulseFrameP1Note(uint16_t value) {
  return (uint8_t)(value & 0x7FU);
}

static inline uint8_t pulseFrameP2Note(uint16_t value) {
  return (uint8_t)((value >> 8) & 0x7FU);
}

static inline bool pulseFrameP1Gate(uint16_t value) {
  return (value & 0x0080U) != 0;
}

static inline bool pulseFrameP2Gate(uint16_t value) {
  return (value & 0x8000U) != 0;
}

static bool canReplacePulseFrame(uint16_t pendingValue, uint16_t nextValue) {
  const bool pendingP1Gate = pulseFrameP1Gate(pendingValue);
  const bool pendingP2Gate = pulseFrameP2Gate(pendingValue);
  const bool nextP1Gate = pulseFrameP1Gate(nextValue);
  const bool nextP2Gate = pulseFrameP2Gate(nextValue);

  if (pendingP1Gate != nextP1Gate || pendingP2Gate != nextP2Gate) {
    return false;
  }

  // Preserve any audible note change while a voice is already gated.
  // Only silent-note updates (gate low on both sides) are safe to collapse.
  if (pendingP1Gate && pulseFrameP1Note(pendingValue) != pulseFrameP1Note(nextValue)) {
    return false;
  }
  if (pendingP2Gate && pulseFrameP2Note(pendingValue) != pulseFrameP2Note(nextValue)) {
    return false;
  }

  return true;
}

static bool replacePendingEvent(uint8_t regId, uint16_t value) {
  // Do not mutate queueHead: the NES may already be halfway through reading it.
  bool found = false;
  uint8_t foundIndex = 0;

  // State frames carry both voices together, but replacing an audible edge here
  // erases short notes at higher tempos. Only collapse frames when they differ
  // in silent state, not in sounding note/gate information.
  if (regId == REG_PULSE_FRAME) {
    for (uint8_t i = 1; i < queueCount; ++i) {
      const uint8_t index = queueIndexAt(i);
      if (queueBuf[index].regId == regId) {
        found = true;
        foundIndex = index;
      }
    }
    if (found && canReplacePulseFrame(queueBuf[foundIndex].value, value)) {
      queueBuf[foundIndex].value = value;
      ++midiFramesReplaced;
      return true;
    }
    return false;
  }

  // Note Off edges must remain ordered. Replacing them can erase short notes
  // when P1 and P2 cross in the queue.
  if (!compactGate((uint8_t)value)) {
    return false;
  }

  for (uint8_t i = 1; i < queueCount; ++i) {
    const uint8_t index = queueIndexAt(i);
    if (queueBuf[index].regId == regId) {
      found = true;
      foundIndex = index;
    }
  }

  if (found && compactGate((uint8_t)queueBuf[foundIndex].value) == compactGate((uint8_t)value)) {
    queueBuf[foundIndex].value = value;
    ++midiFramesReplaced;
    return true;
  }

  return false;
}

static bool enqueueOrReplaceEvent(const ProtoEvent& evt) {
  if (replacePendingEvent(evt.regId, evt.value)) {
    return true;
  }
  return enqueueEvent(evt);
}

static bool popEvent() {
  const ProtoEvent* evt = frontEvent();
  if (!evt) return false;
  if (evt->regId == REG_P1_NOTE) {
    return ackVoiceEvent(VOICE_P1);
  }
  if (evt->regId == REG_P2_NOTE) {
    return ackVoiceEvent(VOICE_P2);
  }
  if (evt->regId == REG_TRI_NOTE) {
    return ackVoiceEvent(VOICE_TRI);
  }
  if (evt->regId == REG_NOI_NOTE) {
    return ackVoiceEvent(VOICE_NOISE);
  }
  return false;
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

static bool commitControlState(const ControlState& nextState, const char* reason) {
  const uint8_t needed = countControlChanges(currentControlState, nextState, false);
  if (queueCount > (uint8_t)(QUEUE_CAPACITY - needed)) {
    overflowLatched = true;
    logState(reason);
    return false;
  }
  if (!enqueueControlState(currentControlState, nextState, false)) {
    logState(reason);
    return false;
  }
  currentControlState = nextState;
  logState(reason);
  return true;
}

static uint8_t clampPlayableNote(uint8_t note) {
  if (note < 24U) return 24U;
  if (note > 95U) return 95U;
  return note;
}

static uint8_t mapNoiseIndex(uint8_t midiNote) {
  uint8_t clamped = midiNote;
  if (clamped < 24U) clamped = 24U;
  if (clamped > 87U) clamped = 87U;
  return (uint8_t)((clamped - 24U) >> 2);
}

static uint8_t mapVoiceNote(uint8_t voiceIndex, uint8_t midiNote) {
  if (voiceIndex == VOICE_NOISE) {
    return mapNoiseIndex(midiNote);
  }
  return clampPlayableNote(midiNote);
}

static void applyVoiceToState(ControlState& state, uint8_t voiceIndex, uint8_t nesNote, bool gate, bool retrigger) {
  if (voiceIndex == VOICE_P1) {
    state.p1Note = nesNote;
    state.p1Gate = gate ? 1U : 0U;
    if (retrigger) ++state.p1Trig;
  } else if (voiceIndex == VOICE_P2) {
    state.p2Note = nesNote;
    state.p2Gate = gate ? 1U : 0U;
    if (retrigger) ++state.p2Trig;
  } else if (voiceIndex == VOICE_TRI) {
    state.triNote = nesNote;
    state.triGate = gate ? 1U : 0U;
    if (retrigger) ++state.triTrig;
  } else {
    state.noiseNote = nesNote;
    state.noiseGate = gate ? 1U : 0U;
  }
}

static uint8_t voiceFromChannel(uint8_t channel) {
  if (channel == CH_P1) return VOICE_P1;
  if (channel == CH_P2) return VOICE_P2;
  if (channel == CH_TRI) return VOICE_TRI;
  if (channel == CH_NOISE) return VOICE_NOISE;
  return VOICE_NONE;
}

static uint8_t regForVoice(uint8_t voiceIndex) {
  return (voiceIndex == VOICE_P1) ? REG_P1_NOTE :
         (voiceIndex == VOICE_P2) ? REG_P2_NOTE :
         (voiceIndex == VOICE_TRI) ? REG_TRI_NOTE : REG_NOI_NOTE;
}

static void logMidiPressure(const char* reason) {
  if (overflowLatched) {
    if (!overflowPressureLogged) {
      requestHeartbeatBurst(HEARTBEAT_BURST_COUNT);
      logState(reason);
      overflowPressureLogged = true;
      lastMidiPressureLogMs = millis();
    }
    return;
  }
  overflowPressureLogged = false;
}

static bool stackContains(const MidiVoice& voice, uint8_t midiNote) {
  for (uint8_t i = 0; i < voice.stackCount; ++i) {
    if (voice.stack[i] == midiNote) {
      return true;
    }
  }
  return false;
}

static bool pushVoiceNote(MidiVoice& voice, uint8_t midiNote) {
  if (stackContains(voice, midiNote)) {
    return true;
  }
  if (voice.stackCount >= (sizeof(voice.stack) / sizeof(voice.stack[0]))) {
    for (uint8_t i = 1; i < voice.stackCount; ++i) {
      voice.stack[i - 1U] = voice.stack[i];
    }
    --voice.stackCount;
  }
  voice.stack[voice.stackCount++] = midiNote;
  return true;
}

static bool removeVoiceNote(MidiVoice& voice, uint8_t midiNote) {
  for (uint8_t i = 0; i < voice.stackCount; ++i) {
    if (voice.stack[i] == midiNote) {
      for (uint8_t j = (uint8_t)(i + 1U); j < voice.stackCount; ++j) {
        voice.stack[j - 1U] = voice.stack[j];
      }
      --voice.stackCount;
      return true;
    }
  }
  return false;
}

static uint8_t topVoiceNote(const MidiVoice& voice) {
  return voice.stackCount ? voice.stack[voice.stackCount - 1U] : 0U;
}

static void clearVoiceStack(MidiVoice& voice) {
  voice.stackCount = 0;
}

static bool enqueueCompactVoice(uint8_t voiceIndex, uint8_t nesNote, bool gate, const char* reason) {
  uint8_t compactValue = (uint8_t)(nesNote & 0x7FU);

  if (gate) {
    compactValue |= COMPACT_GATE_BIT;
  }

  if (!transportPrimed) {
    latchPreAckVoice(voiceIndex, compactValue);
    ++midiFramesEnqueued;
    ++midiPreAckLatched;
    lastMidiEventMs = millis();
    return true;
  }

  if (!enqueueVoiceEvent(voiceIndex, compactValue)) {
    overflowLatched = true;
    logMidiPressure(reason);
    return false;
  }

  ++midiFramesEnqueued;
  lastMidiEventMs = millis();
  logMidiPressure(reason);
  return true;
}

static void handleMidiNoteOff(uint8_t channel, uint8_t midiNote);

static void handleMidiNoteOn(uint8_t channel, uint8_t midiNote, uint8_t velocity) {
  uint8_t voiceIndex;
  MidiVoice* voice;
  uint8_t topMidiNote;
  uint8_t nesNote;

  if (velocity == 0) {
    handleMidiNoteOff(channel, midiNote);
    return;
  }

  voiceIndex = voiceFromChannel(channel);
  if (voiceIndex == VOICE_NONE) return;
  voice = &midiVoices[voiceIndex];
  pushVoiceNote(*voice, midiNote);
  topMidiNote = topVoiceNote(*voice);
  if (topMidiNote == 0U) return;

  nesNote = mapVoiceNote(voiceIndex, topMidiNote);

  if (enqueueCompactVoice(voiceIndex, nesNote, true, "MIDI note on")) {
    voice->active = true;
    voice->midiNote = topMidiNote;
    voice->nesNote = nesNote;
  }
}

static void handleMidiNoteOff(uint8_t channel, uint8_t midiNote) {
  uint8_t voiceIndex;
  MidiVoice* voice;
  uint8_t resumeMidiNote;
  uint8_t resumeNesNote;

  voiceIndex = voiceFromChannel(channel);
  if (voiceIndex == VOICE_NONE) return;
  voice = &midiVoices[voiceIndex];
  if (!removeVoiceNote(*voice, midiNote)) {
    ++midiIgnoredNoteOffs;
    return;
  }

  if (voice->stackCount != 0U) {
    resumeMidiNote = topVoiceNote(*voice);
    resumeNesNote = mapVoiceNote(voiceIndex, resumeMidiNote);
    if (enqueueCompactVoice(voiceIndex, resumeNesNote, true, "MIDI note resume")) {
      voice->active = true;
      voice->midiNote = resumeMidiNote;
      voice->nesNote = resumeNesNote;
      ++midiResumedNotes;
    }
    return;
  }

  if (!voice->active) {
    return;
  }

  if (enqueueCompactVoice(voiceIndex, voice->nesNote, false, "MIDI note off")) {
    voice->active = false;
  }
}

static void handleAllNotesOff(uint8_t channel) {
  const bool clearP1 = (channel == CH_P1 || channel == CH_GLOBAL);
  const bool clearP2 = (channel == CH_P2 || channel == CH_GLOBAL);
  const bool clearTri = (channel == CH_TRI || channel == CH_GLOBAL);
  const bool clearNoise = (channel == CH_NOISE || channel == CH_GLOBAL);

  if (!clearP1 && !clearP2 && !clearTri && !clearNoise) return;

  if (clearP1 && midiVoices[VOICE_P1].active) {
    clearVoiceStack(midiVoices[VOICE_P1]);
    if (enqueueCompactVoice(VOICE_P1, midiVoices[VOICE_P1].nesNote, false, "MIDI all off")) {
      midiVoices[VOICE_P1].active = false;
    }
  }
  if (clearP2 && midiVoices[VOICE_P2].active) {
    clearVoiceStack(midiVoices[VOICE_P2]);
    if (enqueueCompactVoice(VOICE_P2, midiVoices[VOICE_P2].nesNote, false, "MIDI all off")) {
      midiVoices[VOICE_P2].active = false;
    }
  }
  if (clearTri && midiVoices[VOICE_TRI].active) {
    clearVoiceStack(midiVoices[VOICE_TRI]);
    if (enqueueCompactVoice(VOICE_TRI, midiVoices[VOICE_TRI].nesNote, false, "MIDI all off")) {
      midiVoices[VOICE_TRI].active = false;
    }
  }
  if (clearNoise && midiVoices[VOICE_NOISE].active) {
    clearVoiceStack(midiVoices[VOICE_NOISE]);
    if (enqueueCompactVoice(VOICE_NOISE, midiVoices[VOICE_NOISE].nesNote, false, "MIDI all off")) {
      midiVoices[VOICE_NOISE].active = false;
    }
  }
}

static uint8_t midiExpectedLength(uint8_t status) {
  const uint8_t type = (uint8_t)(status & 0xF0U);
  if (type == 0xC0U || type == 0xD0U) return 1;
  if (type >= 0x80U && type <= 0xE0U) return 2;
  return 0;
}

static void handleMidiMessage(uint8_t status, uint8_t data1, uint8_t data2) {
  const uint8_t type = (uint8_t)(status & 0xF0U);
  const uint8_t channel = (uint8_t)((status & 0x0FU) + 1U);
  if (type == 0x90U) {
    handleMidiNoteOn(channel, data1, data2);
  } else if (type == 0x80U) {
    handleMidiNoteOff(channel, data1);
  } else if (type == 0xB0U && (data1 == 120U || data1 == 123U)) {
    handleAllNotesOff(channel);
  }
}

static void handleMidiByte(uint8_t b) {
  if (b & 0x80U) {
    if (b >= 0xF8U) return;
    if (b < 0xF0U) {
      midiRunningStatus = b;
      midiExpectedData = midiExpectedLength(b);
      midiDataCount = 0;
    } else {
      midiRunningStatus = 0;
      midiExpectedData = 0;
      midiDataCount = 0;
    }
    return;
  }

  if (midiRunningStatus == 0 || midiExpectedData == 0) return;

  midiData[midiDataCount++] = (uint8_t)(b & 0x7FU);
  if (midiDataCount >= midiExpectedData) {
    handleMidiMessage(midiRunningStatus, midiData[0], midiExpectedData > 1 ? midiData[1] : 0);
    midiDataCount = 0;
  }
}

static void serviceMidiInput() {
  while (Serial1.available() > 0) {
    handleMidiByte((uint8_t)Serial1.read());
  }
}

static void seedQueue() {
  clearQueue();
  currentControlState = kInitialState;
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    midiVoices[voiceIndex].active = false;
    midiVoices[voiceIndex].stackCount = 0;
  }
  midiFramesEnqueued = 0;
  midiFramesAcked = 0;
  midiFramesReplaced = 0;
  midiIgnoredNoteOffs = 0;
  midiResumedNotes = 0;
  midiPreAckLatched = 0;
  lastMidiPressureLogMs = 0;
  overflowPressureLogged = false;
  queueHighWater = 0;
}

static void resetLiveSource() {
  seedQueue();
  sawFirstAck = false;
  transportPrimed = false;
  warmupDirtyMask = 0;
  firstAckMs = 0;
  midiRunningStatus = 0;
  midiExpectedData = 0;
  midiDataCount = 0;
}

static void updateReloadDetector(uint8_t out, bool outChanged) {
  if (!ENABLE_RELOAD_DETECTOR) {
    return;
  }
  if (!g245Enabled || !outChanged || (totalPendingCount() != 0U)) {
    return;
  }

  if (out == kArmSequence[reloadIndex]) {
    ++reloadIndex;
    if (reloadIndex >= (sizeof(kArmSequence) / sizeof(kArmSequence[0]))) {
      reloadIndex = 0;
      resetLiveSource();
      writePattern(STATUS_VALID_ONLY);
      ++reloadCount;
      logState("queue reload");
    }
  } else if (out == kArmSequence[0]) {
    reloadIndex = 1;
  } else {
    reloadIndex = 0;
  }
}

static void applyOpcode(uint8_t out, bool outChanged) {
  if (!outChanged) {
    return;
  }

  uint8_t nextPattern = currentPattern;

  switch (out) {
    case OPCODE_IDLE:
      nextPattern = STATUS_VALID_ONLY;
      break;
    case OPCODE_READ_STATUS:
      if (outChanged) {
        const bool hadPending = latchedBatchActive &&
                                (latchedP1Pending || latchedP2Pending || latchedAuxPending);
        if (hadPending && !sawFirstAck) {
          sawFirstAck = true;
          firstAckMs = millis();
          resetBusSniffStats();
        }
        if (latchedBatchActive) {
          ackBatchEvents();
        }
      }
      nextPattern = latchAndBuildStatusPattern();
      break;
    case OPCODE_READ_P1_LO:
      nextPattern = buildP1LoPattern();
      break;
    case OPCODE_READ_P1_HI:
      nextPattern = buildP1HiPattern();
      break;
    case OPCODE_READ_P2_LO:
      nextPattern = buildP2LoPattern();
      break;
    case OPCODE_READ_P2_HI:
      nextPattern = buildP2HiPattern();
      break;
    case OPCODE_READ_AUX_LO:
      nextPattern = buildAuxLoPattern();
      break;
    case OPCODE_READ_AUX_HI:
      nextPattern = buildAuxHiPattern();
      break;
    default:
      nextPattern = STATUS_VALID_ONLY;
      break;
  }

  if (nextPattern != currentPattern) {
    writePattern(nextPattern);
    if (VERBOSE_BUS_LOGS) logState("D change");
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  disable245Safe();
  setupJoypadOutputs();
  setupOutInputs();
  resetLiveSource();
  resetBusSniffStats();
  writePattern(STATUS_VALID_ONLY);

  Serial.begin(115200);
  Serial1.setRX(PIN_MIDI_RX);
  Serial1.begin(MIDI_BAUD);
  delay(250);
  Serial.println();
  Serial.println(F("PicoNesV2B_Step2P1P2TriEdgeMidi start"));
  Serial.println(F("DIN MIDI on GP1, V18 map: CH12=P1 CH13=P2 CH14=TRI"));
  Serial.print(F("FW rev: "));
  Serial.println(FW_REV);

  currentOut = readOutBits();
  committedOut = currentOut;
  outCandidateSinceUs = micros();
  lastCommittedOutChangeUs = outCandidateSinceUs;
  if (ENABLE_BUS_SNIFF && pinConfigured(PIN_OE2)) {
    lastOe2Level = digitalRead(PIN_OE2);
    attachInterrupt(digitalPinToInterrupt(PIN_OE2), oe2EdgeIsr, CHANGE);
  }
  bootMs = millis();
  logState("boot");
}

void loop() {
  const uint32_t nowMs = millis();
  const uint32_t nowUs = micros();
  const uint8_t sampledOut = readOutBits();
  const uint32_t settleUs = g245Enabled ? ACTIVE_OUT_SETTLE_US : ARM_OUT_SETTLE_US;
  const bool auxConflictActive = auxVoicesContending() && midiActivityRecent(nowMs);

  if (auxConflictActive && !lastAuxConflictActive) {
    requestHeartbeatBurst(HEARTBEAT_BURST_COUNT);
  }
  lastAuxConflictActive = auxConflictActive;

  if (sampledOut != currentOut) {
    currentOut = sampledOut;
    outCandidateSinceUs = nowUs;
  }

  bool outChanged = false;
  uint8_t out = committedOut;
  if ((currentOut != committedOut) &&
      ((uint32_t)(nowUs - outCandidateSinceUs) >= settleUs)) {
    committedOut = currentOut;
    lastCommittedOutChangeUs = nowUs;
    out = committedOut;
    outChanged = true;
    if (VERBOSE_BUS_LOGS) logState("OUT change");
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
          ++armStepCount;
          logState("arm step");
        }
      } else if (out == kArmSequence[0]) {
        armIndex = 1;
        ++armRestartCount;
        logState("arm restart");
      } else if (out != 0x00) {
        armIndex = 0;
        ++armResetCount;
      }
    }
    if (ENABLE_ARM_FALLBACK &&
        !g245Enabled &&
        currentOut != 0x00 &&
        (uint32_t)(nowMs - bootMs) >= ARM_FALLBACK_MS) {
      g245Enabled = true;
      enable245();
      digitalWrite(LED_BUILTIN, HIGH);
      resetLiveSource();
      ++armFallbackCount;
      logState("245 fallback");
      applyOpcode(out, outChanged);
    }
  } else {
    serviceMidiInput();
    updateReloadDetector(out, outChanged);
    applyOpcode(out, outChanged);
  }

  if ((uint32_t)(nowMs - lastHeartbeatMs) >= currentHeartbeatIntervalMs(nowMs)) {
    lastHeartbeatMs = nowMs;
    logState("heartbeat");
    if (heartbeatBurstRemaining != 0U) {
      --heartbeatBurstRemaining;
    }
  }
}
