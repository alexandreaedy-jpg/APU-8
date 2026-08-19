#include <Arduino.h>
#include <SPI.h>

// V2_B / MIDI In P1/P2/TRI/NOISE/DMC + controls
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

static const uint8_t PIN_PANEL_LFO_PITCH = 8;
static const uint8_t PIN_PANEL_LFO_DUTY = 9;
static const uint8_t PIN_PANEL_LFO_AMP = 15;
static const uint8_t PIN_PANEL_VOICE_P1 = 20;
static const uint8_t PIN_PANEL_VOICE_P2 = 21;
static const uint8_t PIN_PANEL_VOICE_TRI = 22;
static const uint8_t PIN_PANEL_VOICE_NOISE = 26;
static const uint8_t PIN_PANEL_VOICE_GLOBAL = 27;
static const uint8_t PIN_PANEL_ARP_ENABLE = 28;

static const uint8_t PIN_MCP_MISO = 16;
static const uint8_t PIN_MCP_CS = 17;
static const uint8_t PIN_MCP_SCK = 18;
static const uint8_t PIN_MCP_MOSI = 19;

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
static const uint8_t STATUS_AUX_TYPE_NOISE_SHORT = 0x00;
static const uint8_t STATUS_AUX_TYPE_TRI_FULL = 0x08;
static const uint8_t STATUS_AUX_TYPE_DMC_FULL = 0x10;
static const uint8_t STATUS_AUX_TYPE_NOISE_FULL = 0x18;
static const uint8_t STATUS_IDLE_PATTERN = 0x00;

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
static const bool ENABLE_PANEL_RAW_LOG = false;
static const char FW_REV[] = "t58-v2b-step5ac-arppressure";

static const uint32_t PANEL_SCAN_INTERVAL_MS = 24U;
static const uint32_t PANEL_ACTIVE_LOCK_MS = 120U;
static const uint32_t NOTE_COHORT_WINDOW_US = 0U;
static const uint32_t PANEL_CONTROL_INJECT_INTERVAL_MS = 16U;
static const uint32_t PANEL_ARP_TIME_STABLE_MS = 40U;
static const uint8_t PANEL_ANALOG_CHANNELS = 8U;
static const uint8_t PANEL_CONTROL_RELEASE_Q = 0U;
static const uint16_t PANEL_ADSR_RAW_DEADBAND = 16U;
static const uint16_t PANEL_LFO_RAW_DEADBAND = 20U;
static const uint16_t PANEL_DEFAULT_RAW_DEADBAND = 32U;
static const uint8_t PANEL_SELECTOR_STABLE_SCANS = 3U;
static const bool PANEL_SWITCH_ACTIVE_LOW = true;
static const uint8_t PANEL_LFO_TARGET_PITCH = 0U;
static const uint8_t PANEL_LFO_TARGET_DUTY = 1U;
static const uint8_t PANEL_LFO_TARGET_AMP = 2U;
static const uint8_t PANEL_LFO_TARGET_NONE = 0xFFU;
static const uint8_t PANEL_TARGET_GLOBAL = 5U;
static const uint8_t LFO_WAVE_SINE = 2U;

static const uint8_t MCP_CH_ATTACK = 0U;
static const uint8_t MCP_CH_VOLUME = 1U;
static const uint8_t MCP_CH_DECAY = 2U;
static const uint8_t MCP_CH_RELEASE = 3U;
static const uint8_t MCP_CH_LFO_DEPTH = 4U;
static const uint8_t MCP_CH_LFO_RATE = 5U;
static const uint8_t MCP_CH_DUTY = 6U;
static const uint8_t MCP_CH_ARP_TIME = 7U;

static const uint8_t kArmSequence[] = {0x04, 0x02, 0x01, 0x07};
static const uint8_t VOICE_EDGE_QUEUE_CAPACITY = 48;

struct ProtoEvent {
  uint8_t regId;
  uint16_t value;
};

struct VoiceEdgeQueue {
  uint8_t values[VOICE_EDGE_QUEUE_CAPACITY];
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

struct VoiceControlState {
  uint8_t attack;
  uint8_t decay;
  uint8_t volume;
  uint8_t release;
  uint8_t duty;
  uint8_t lfoPitchDepth;
  uint8_t lfoRate;
  uint8_t lfoDutyDepth;
  uint8_t lfoAmpDepth;
  uint8_t lfoDelay;
  uint8_t lfoWave;
  uint8_t samplePitch;
  uint8_t modeFlags;
};

struct MidiVoice {
  bool active;
  uint8_t midiNote;
  uint8_t nesNote;
  uint8_t stackCount;
  uint8_t stack[4];
};

struct ArpState {
  bool enabled;
  uint8_t divisionIndex;
  uint8_t stepIndex;
  uint8_t clockCounter;
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

// V18/V2B MIDI map. Keep pulse/tri/noise fixed and reserve CH11 for DMC sample triggers.
static const uint8_t CH_DMC = 11;
static const uint8_t CH_P1 = 12;
static const uint8_t CH_P2 = 13;
static const uint8_t CH_TRI = 14;
static const uint8_t CH_NOISE = 15;
static const uint8_t CH_GLOBAL = 16;
static const uint8_t CC_ATTACK = 20;
static const uint8_t CC_DECAY = 21;
static const uint8_t CC_VOLUME = 22;
static const uint8_t CC_RELEASE = 23;
static const uint8_t CC_DUTY = 24;
static const uint8_t CC_LFO_DEPTH = 25;
static const uint8_t CC_LFO_RATE = 26;
static const uint8_t CC_ARP_ENABLE = 27;
static const uint8_t CC_ARP_DIV = 28;
static const uint8_t CC_LFO_DUTY_DEPTH = 29;
static const uint8_t CC_LFO_AMP_DEPTH = 30;
static const uint8_t CC_LFO_DELAY = 34;
static const uint8_t CC_LFO_WAVE = 35;
static const uint8_t CC_DMC_PITCH = 36;

static const uint8_t VOICE_P1 = 0;
static const uint8_t VOICE_P2 = 1;
static const uint8_t VOICE_TRI = 2;
static const uint8_t VOICE_NOISE = 3;
static const uint8_t VOICE_DMC = 4;
static const uint8_t VOICE_CTRL = 5;
static const uint8_t VOICE_NONE = 0xFF;
static const uint8_t VOICE_COUNT = 5;

static const uint8_t CTRL_ATTACK = 0;
static const uint8_t CTRL_DECAY = 1;
static const uint8_t CTRL_VOLUME = 2;
static const uint8_t CTRL_RELEASE = 3;
static const uint8_t CTRL_DUTY = 4;
static const uint8_t CTRL_LFO_DEPTH = 5;
static const uint8_t CTRL_LFO_RATE = 6;
static const uint8_t CTRL_LFO_DUTY_DEPTH = 7;
static const uint8_t CTRL_LFO_AMP_DEPTH = 8;
static const uint8_t CTRL_LFO_DELAY = 9;
static const uint8_t CTRL_LFO_WAVE = 10;
static const uint8_t CTRL_DMC_TRIGGER = 11;
static const uint8_t CTRL_MODE_FLAGS = 12;
static const uint8_t CTRL_SLOT_COUNT = 13;

static const uint8_t AUX_CTRL_ATTACK = 0x20;
static const uint8_t AUX_CTRL_DECAY = 0x30;
static const uint8_t AUX_CTRL_VOLUME = 0x40;
static const uint8_t AUX_CTRL_RELEASE = 0x50;
static const uint8_t AUX_CTRL_DUTY = 0x60;
static const uint8_t AUX_CTRL_LFO_DEPTH = 0x70;
static const uint8_t AUX_CTRL_LFO_RATE = 0x90;
static const uint8_t VOICE_CTRL_META_BASE = 0xF0;
static const uint8_t VOICE_CTRL_VALUE_BASE = 0xE0;
static const uint8_t DMC_CTRL_META_BASE = 0x70;
static const uint8_t DMC_CTRL_VALUE_BASE = 0x60;
static const uint8_t DMC_TRIGGER_META_LO = 0xFD;
static const uint8_t DMC_TRIGGER_META_HI = 0xFE;
static const uint8_t DMC_TRIGGER_VALUE_BASE = 0xE0;
static const uint8_t VOICE_BEND_META_BASE = 0xA0;
static const uint8_t VOICE_BEND_VALUE_BASE = 0xB0;
static const uint8_t VOICE_GLIDE_META_BASE = 0xC0;
static const uint8_t VOICE_GLIDE_VALUE_BASE = 0xD0;

static const uint8_t VOICE_CODE_P1 = 0;
static const uint8_t VOICE_CODE_P2 = 1;
static const uint8_t VOICE_CODE_TRI = 2;
static const uint8_t VOICE_CODE_NOISE = 3;
static const uint8_t VOICE_CODE_DMC = 4;

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
static const uint8_t REG_DMC_NOTE = 0x10;
static const uint8_t DMC_SAMPLE_COUNT = 26;
static const uint8_t DMC_MAX_PENDING_EVENTS = 4;
static const uint8_t DMC_ARP_PRESSURE_DROP_Q = 6;
static const uint8_t DMC_AUX_DEFERRAL_LIMIT = 2;
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

static const VoiceControlState kInitialVoiceControls[VOICE_COUNT] = {
  {0, 0, 15, 0, 2, 0, 0, 0, 0, 0, LFO_WAVE_SINE, 8, 0},
  {0, 0, 15, 0, 2, 0, 0, 0, 0, 0, LFO_WAVE_SINE, 8, 0},
  {0, 0, 15, 0, 0, 0, 0, 0, 0, 0, LFO_WAVE_SINE, 8, 0},
  {0, 0, 15, 0, 8, 0, 0, 0, 0, 0, LFO_WAVE_SINE, 8, 0},
  {0, 0, 15, 0, 0, 0, 0, 0, 0, 0, LFO_WAVE_SINE, 8, 0}
};

static const uint8_t QUEUE_CAPACITY = 32;
static ProtoEvent queueBuf[QUEUE_CAPACITY];
static uint8_t queueHead = 0;
static uint8_t queueTail = 0;
static uint8_t queueCount = 0;
static VoiceEdgeQueue voiceQueues[VOICE_COUNT];
static bool preAckVoiceValid[VOICE_COUNT] = {false, false, false, false, false};
static uint8_t preAckVoiceValue[VOICE_COUNT] = {0, 0, 0, 0, 0};
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
static uint8_t controlDirtyMask = 0;
static uint8_t controlScanStart = 0;
static bool controlSchedulePending[VOICE_COUNT][CTRL_SLOT_COUNT];
static uint8_t controlScheduleValue[VOICE_COUNT][CTRL_SLOT_COUNT];
static uint8_t controlScheduleVoiceCursor = 0;
static uint8_t controlScheduleControlCursor = 0;
static uint32_t controlSchedulePendingWrites = 0;
static uint32_t controlScheduleInjectedPairs = 0;
static uint32_t dmcDroppedTriggers = 0;
static uint32_t lastControlScheduleInjectMs = 0;
static uint8_t dmcAuxDeferrals = 0;
static bool noteCohortActive = false;
static uint32_t noteCohortStartUs = 0;
static bool noteCohortPending[VOICE_COUNT] = {false, false, false, false, false};
static uint8_t noteCohortValue[VOICE_COUNT] = {0, 0, 0, 0, 0};
static uint32_t noteCohortFlushes = 0;

static bool g245Enabled = false;
static bool overflowLatched = false;
static uint8_t currentPattern = STATUS_IDLE_PATTERN;
static volatile uint8_t currentOut = 0xFF;
static volatile uint8_t committedOut = 0xFF;
static uint8_t armIndex = 0;
static uint8_t reloadIndex = 0;
static ControlState currentControlState = kInitialState;
static VoiceControlState voiceControls[VOICE_COUNT] = {
  kInitialVoiceControls[0],
  kInitialVoiceControls[1],
  kInitialVoiceControls[2],
  kInitialVoiceControls[3],
  kInitialVoiceControls[4]
};
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
static uint32_t midiControlReceived = 0;
static uint32_t midiControlApplied = 0;
static uint32_t panelControlApplied = 0;
static uint32_t panelAnalogSuppressed = 0;
static uint32_t midiClockTicks = 0;
static uint32_t armStepCount = 0;
static uint32_t armRestartCount = 0;
static uint32_t armResetCount = 0;
static uint32_t armFallbackCount = 0;
static uint32_t reloadCount = 0;
static uint8_t queueHighWater = 0;
static uint16_t panelFiltered[PANEL_ANALOG_CHANNELS] = {0,0,0,0,0,0,0,0};
static uint16_t panelLastRaw[PANEL_ANALOG_CHANNELS] = {0,0,0,0,0,0,0,0};
static uint8_t panelLastQuant[PANEL_ANALOG_CHANNELS] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static uint8_t panelCurrentQuant[PANEL_ANALOG_CHANNELS] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static bool panelAnalogPrimed = false;
static bool panelSpiReady = false;
static uint8_t panelLastTarget = 0xFFU;
static uint8_t panelLastLfoTarget = 0xFFU;
static bool panelLastArpSwitch = false;
static uint8_t panelLastVoiceMask = 0U;
static uint8_t panelLastLfoMask = 0U;
static uint8_t panelCandidateTarget = 0xFFU;
static uint8_t panelCandidateLfoTarget = 0xFFU;
static uint8_t panelSelectorStableCount = 0U;
static uint8_t panelLastAppliedChannel = 0xFFU;
static uint8_t panelLastAppliedValue = 0xFFU;
static bool panelLastAppliedArpTimeWasDelay = false;
static uint8_t panelActiveAnalogChannel = 0xFFU;
static uint32_t panelActiveAnalogUntilMs = 0;
static uint8_t panelArpTimeCandidateValue = 0xFFU;
static bool panelArpTimeCandidateIsDelay = false;
static uint32_t panelArpTimeCandidateSinceMs = 0;
static uint32_t lastPanelScanMs = 0;
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
  {false, 0, 0, 0, {0, 0, 0, 0}},
  {false, 0, 0, 0, {0, 0, 0, 0}}
};
static ArpState arpStates[VOICE_COUNT] = {
  {false, 2, 0, 0},
  {false, 2, 0, 0},
  {false, 2, 0, 0},
  {false, 2, 0, 0},
  {false, 2, 0, 0}
};
static bool midiClockRunning = false;
static const uint8_t kArpClocksPerStep[4] = {24U, 12U, 6U, 3U};

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
    case VOICE_DMC: return VOICE_CODE_DMC;
    default: return VOICE_CODE_P1;
  }
}

static const __FlashStringHelper* voiceNameForIndex(uint8_t voiceIndex) {
  switch (voiceIndex) {
    case VOICE_P1: return F("P1");
    case VOICE_P2: return F("P2");
    case VOICE_TRI: return F("TRI");
    case VOICE_NOISE: return F("NOI");
    case VOICE_DMC: return F("DMC");
    case VOICE_CTRL: return F("CTL");
    default: return F("--");
  }
}

static inline uint8_t controlMaskBit(uint8_t controlIndex) {
  return (uint8_t)(1U << controlIndex);
}

static inline bool controlPending(uint8_t controlIndex) {
  return (controlDirtyMask & controlMaskBit(controlIndex)) != 0U;
}

static inline uint8_t controlPendingCount() {
  return 0;
}

static inline uint8_t pendingCountForVoice(uint8_t voiceIndex) {
  return (uint8_t)(voiceQueues[voiceIndex].count + (preAckVoiceValid[voiceIndex] ? 1U : 0U));
}

static inline uint8_t totalPendingCount() {
  uint8_t total = controlPendingCount();
  for (uint8_t voiceIndex = VOICE_P1; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    total = (uint8_t)(total + pendingCountForVoice(voiceIndex));
  }
  return total;
}

static inline uint8_t musicalPendingCount() {
  uint8_t total = 0U;
  for (uint8_t voiceIndex = VOICE_P1; voiceIndex <= VOICE_NOISE; ++voiceIndex) {
    total = (uint8_t)(total + pendingCountForVoice(voiceIndex));
  }
  return total;
}

static inline bool anyArpActive() {
  return (arpStates[VOICE_P1].enabled && (midiVoices[VOICE_P1].stackCount != 0U || midiVoices[VOICE_P1].active)) ||
         (arpStates[VOICE_P2].enabled && (midiVoices[VOICE_P2].stackCount != 0U || midiVoices[VOICE_P2].active)) ||
         (arpStates[VOICE_TRI].enabled && (midiVoices[VOICE_TRI].stackCount != 0U || midiVoices[VOICE_TRI].active));
}

static inline bool voiceHasPending(uint8_t voiceIndex) {
  return pendingCountForVoice(voiceIndex) != 0U;
}

static inline bool pulseTransportPending() {
  return voiceHasPending(VOICE_P1) || voiceHasPending(VOICE_P2);
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

static inline bool isVoiceControlMetaByte(uint8_t value) {
  return (value & 0xF0U) == VOICE_CTRL_META_BASE;
}

static inline bool isVoiceControlValueByte(uint8_t value) {
  return (value & 0xF0U) == VOICE_CTRL_VALUE_BASE;
}

static inline bool isDmcControlMetaByte(uint8_t value) {
  return value == DMC_TRIGGER_META_LO || value == DMC_TRIGGER_META_HI;
}

static inline bool isDmcControlValueByte(uint8_t value) {
  return !isDmcControlMetaByte(value) && (value & 0xF0U) == DMC_TRIGGER_VALUE_BASE;
}

static inline bool isVoiceControlByte(uint8_t value) {
  return isVoiceControlMetaByte(value) || isVoiceControlValueByte(value);
}

static inline uint8_t latchedAuxStatusType() {
  if (!latchedAuxPending) {
    return STATUS_AUX_TYPE_NOISE_SHORT;
  }
  if (latchedAuxVoice == VOICE_TRI) {
    return STATUS_AUX_TYPE_TRI_FULL;
  }
  if (latchedAuxVoice == VOICE_DMC) {
    return STATUS_AUX_TYPE_DMC_FULL;
  }
  if (latchedAuxVoice == VOICE_NOISE && isVoiceControlByte(latchedAuxValue)) {
    return STATUS_AUX_TYPE_NOISE_FULL;
  }
  return STATUS_AUX_TYPE_NOISE_SHORT;
}

static inline bool latchedAuxUsesFullByte() {
  return latchedAuxStatusType() != STATUS_AUX_TYPE_NOISE_SHORT;
}

static inline bool controlIsLfo(uint8_t controlIndex) {
  return controlIndex == CTRL_LFO_DEPTH ||
         controlIndex == CTRL_LFO_RATE ||
         controlIndex == CTRL_LFO_DUTY_DEPTH ||
         controlIndex == CTRL_LFO_AMP_DEPTH ||
         controlIndex == CTRL_LFO_DELAY ||
         controlIndex == CTRL_LFO_WAVE;
}

static uint8_t auxVoicePriorityScore(uint8_t voiceIndex) {
  uint8_t score = 0;
  uint8_t pending = pendingCountForVoice(voiceIndex);
  const uint8_t value = frontVoiceValue(voiceIndex);

  if (voiceIndex == VOICE_DMC) {
    if (isDmcControlValueByte(value)) {
      return 15U;
    }
    if (isDmcControlMetaByte(value)) {
      return (uint8_t)(2U + ((pending > 2U) ? 2U : pending));
    }
    if (pending > 3U) {
      pending = 3U;
    }
    score = (uint8_t)(4U + pending);
    if (voiceIndex != slotScanStart) {
      score = (uint8_t)(score + 1U);
    }
    return score;
  }

  if (isVoiceControlValueByte(value)) {
    return 31U;
  }

  if (isVoiceControlMetaByte(value)) {
    if (voiceIndex == VOICE_NOISE) {
      return 28U;
    }
    return 20U;
  }

  if (compactValueGateOn(value)) {
    score = (uint8_t)(score + 8U);
    if (voiceIndex == VOICE_NOISE) {
      score = (uint8_t)(score + 4U);
    }
    if (voiceIndex == VOICE_TRI) {
      score = (uint8_t)(score + 3U);
    }
  }

  if (pending > 3U) {
    pending = 3U;
  }
  score = (uint8_t)(score + 6U + pending);

  if (voiceIndex != slotScanStart) {
    score = (uint8_t)(score + 1U);
  }
  return score;
}

static inline bool auxVoiceSelectable(uint8_t voiceIndex) {
  return voiceIndex == VOICE_TRI || voiceIndex == VOICE_NOISE || voiceIndex == VOICE_DMC;
}

static inline bool auxVoicesContending() {
  uint8_t active = 0;
  if (voiceHasPending(VOICE_TRI)) ++active;
  if (voiceHasPending(VOICE_NOISE)) ++active;
  if (voiceHasPending(VOICE_DMC)) ++active;
  return active >= 2U;
}

static uint8_t nextAuxVoice(uint8_t voiceIndex) {
  switch (voiceIndex) {
    case VOICE_TRI: return VOICE_NOISE;
    case VOICE_NOISE: return VOICE_DMC;
    case VOICE_DMC: return VOICE_TRI;
    default: return VOICE_TRI;
  }
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

static uint8_t nextControlSlot(uint8_t controlIndex) {
  return (uint8_t)((controlIndex + 1U) % CTRL_SLOT_COUNT);
}

static uint8_t packControlAuxValue(uint8_t controlIndex) {
  switch (controlIndex) {
    case CTRL_ATTACK: return (uint8_t)(AUX_CTRL_ATTACK | (currentControlState.attack & 0x0FU));
    case CTRL_DECAY: return (uint8_t)(AUX_CTRL_DECAY | (currentControlState.decay & 0x0FU));
    case CTRL_VOLUME: return (uint8_t)(AUX_CTRL_VOLUME | (currentControlState.sustain & 0x0FU));
    case CTRL_RELEASE: return (uint8_t)(AUX_CTRL_RELEASE | (currentControlState.release & 0x0FU));
    case CTRL_DUTY: return (uint8_t)(AUX_CTRL_DUTY | (currentControlState.duty & 0x03U));
    case CTRL_LFO_DEPTH: return (uint8_t)(AUX_CTRL_LFO_DEPTH | (currentControlState.lfoDepth & 0x0FU));
    case CTRL_LFO_RATE: return (uint8_t)(AUX_CTRL_LFO_RATE | (currentControlState.lfoRate & 0x0FU));
    default: return 0;
  }
}

static bool selectPendingControl(uint8_t* outValue) {
  (void)outValue;
  return false;
}

static bool voiceSupportsControl(uint8_t voiceIndex, uint8_t controlIndex) {
  switch (voiceIndex) {
    case VOICE_P1:
    case VOICE_P2:
      return controlIndex != CTRL_DMC_TRIGGER;
    case VOICE_TRI:
      return controlIndex == CTRL_MODE_FLAGS;
    case VOICE_NOISE:
      return controlIndex == CTRL_ATTACK ||
             controlIndex == CTRL_DECAY ||
             controlIndex == CTRL_VOLUME ||
             controlIndex == CTRL_RELEASE ||
             controlIndex == CTRL_DUTY ||
             controlIndex == CTRL_LFO_DUTY_DEPTH ||
             controlIndex == CTRL_LFO_AMP_DEPTH ||
             controlIndex == CTRL_LFO_RATE ||
             controlIndex == CTRL_LFO_DELAY ||
             controlIndex == CTRL_LFO_WAVE;
    case VOICE_DMC:
      return false;
    default:
      return false;
  }
}

static uint8_t* voiceControlField(uint8_t voiceIndex, uint8_t controlIndex) {
  VoiceControlState* ctrl = &voiceControls[voiceIndex];
  switch (controlIndex) {
    case CTRL_ATTACK: return &ctrl->attack;
    case CTRL_DECAY: return &ctrl->decay;
    case CTRL_VOLUME: return &ctrl->volume;
    case CTRL_RELEASE: return &ctrl->release;
    case CTRL_DUTY: return &ctrl->duty;
    case CTRL_LFO_DEPTH: return &ctrl->lfoPitchDepth;
    case CTRL_LFO_RATE: return &ctrl->lfoRate;
    case CTRL_LFO_DUTY_DEPTH: return &ctrl->lfoDutyDepth;
    case CTRL_LFO_AMP_DEPTH: return &ctrl->lfoAmpDepth;
    case CTRL_LFO_DELAY: return &ctrl->lfoDelay;
    case CTRL_LFO_WAVE: return &ctrl->lfoWave;
    case CTRL_MODE_FLAGS: return &ctrl->modeFlags;
    default: return nullptr;
  }
}

static uint8_t voiceControlMetaByte(uint8_t voiceIndex, uint8_t controlIndex) {
  if (voiceIndex == VOICE_DMC) {
    return DMC_TRIGGER_META_LO;
  }
  return (uint8_t)(VOICE_CTRL_META_BASE | (controlIndex & 0x0FU));
}

static uint8_t voiceControlValueByte(uint8_t voiceIndex, uint8_t value) {
  if (voiceIndex == VOICE_DMC) {
    return (uint8_t)(DMC_TRIGGER_VALUE_BASE | (value & 0x0FU));
  }
  return (uint8_t)(VOICE_CTRL_VALUE_BASE | (value & 0x0FU));
}

static bool replacePendingVoiceControlValue(uint8_t voiceIndex, uint8_t meta, uint8_t payload) {
  VoiceEdgeQueue& queue = voiceQueues[voiceIndex];
  const uint8_t capacity = (uint8_t)(sizeof(queue.values) / sizeof(queue.values[0]));
  const uint8_t valueBase = (voiceIndex == VOICE_DMC) ? DMC_TRIGGER_VALUE_BASE : VOICE_CTRL_VALUE_BASE;
  bool found = false;
  uint8_t foundIndex = 0;

  if (queue.count < 2U) {
    return false;
  }

  for (uint8_t offset = 0; offset < (uint8_t)(queue.count - 1U); ++offset) {
    const uint8_t metaIndex = (uint8_t)((queue.head + offset) % capacity);
    const uint8_t valueIndex = (uint8_t)((metaIndex + 1U) % capacity);
    if (queue.values[metaIndex] == meta && (queue.values[valueIndex] & 0xF0U) == valueBase) {
      found = true;
      foundIndex = valueIndex;
    }
  }

  if (!found) {
    return false;
  }

  queue.values[foundIndex] = payload;
  ++midiFramesReplaced;
  return true;
}

static bool enqueueVoiceControlPairFront(uint8_t voiceIndex, uint8_t meta, uint8_t payload) {
  VoiceEdgeQueue& queue = voiceQueues[voiceIndex];
  const uint8_t capacity = (uint8_t)(sizeof(queue.values) / sizeof(queue.values[0]));

  if (queue.count > (uint8_t)(capacity - 2U)) {
    return false;
  }

  if (queue.count != 0U) {
    const uint8_t front = queue.values[queue.head];
    if ((voiceIndex == VOICE_DMC && isDmcControlValueByte(front)) ||
        (voiceIndex != VOICE_DMC && isVoiceControlValueByte(front))) {
      return false;
    }
  }

  queue.head = (uint8_t)((queue.head + capacity - 1U) % capacity);
  queue.values[queue.head] = payload;
  ++queue.count;
  queue.head = (uint8_t)((queue.head + capacity - 1U) % capacity);
  queue.values[queue.head] = meta;
  ++queue.count;
  touchQueueHighWater();
  return true;
}

static bool promotePendingVoiceControlPairFront(uint8_t voiceIndex, uint8_t meta, uint8_t payload) {
  VoiceEdgeQueue& queue = voiceQueues[voiceIndex];
  const uint8_t capacity = (uint8_t)(sizeof(queue.values) / sizeof(queue.values[0]));
  const uint8_t valueBase = (voiceIndex == VOICE_DMC) ? DMC_TRIGGER_VALUE_BASE : VOICE_CTRL_VALUE_BASE;
  uint8_t linear[VOICE_EDGE_QUEUE_CAPACITY];
  uint8_t rebuilt[VOICE_EDGE_QUEUE_CAPACITY];
  bool found = false;
  uint8_t foundOffset = 0;

  if (queue.count < 2U) {
    return false;
  }

  for (uint8_t offset = 0; offset < queue.count; ++offset) {
    linear[offset] = queue.values[(queue.head + offset) % capacity];
  }

  for (uint8_t offset = 0; offset < (uint8_t)(queue.count - 1U); ++offset) {
    if (linear[offset] == meta && (linear[offset + 1U] & 0xF0U) == valueBase) {
      found = true;
      foundOffset = offset;
    }
  }

  if (!found) {
    return false;
  }

  if (foundOffset == 0U ||
      (voiceIndex == VOICE_DMC && isDmcControlValueByte(linear[0])) ||
      (voiceIndex != VOICE_DMC && isVoiceControlValueByte(linear[0]))) {
    queue.values[(queue.head + foundOffset + 1U) % capacity] = payload;
    ++midiFramesReplaced;
    return true;
  }

  uint8_t writeIndex = 0;
  rebuilt[writeIndex++] = meta;
  rebuilt[writeIndex++] = payload;
  for (uint8_t offset = 0; offset < queue.count; ++offset) {
    if (offset == foundOffset) {
      ++offset;
      continue;
    }
    rebuilt[writeIndex++] = linear[offset];
  }

  queue.head = 0;
  queue.tail = (uint8_t)(queue.count % capacity);
  for (uint8_t offset = 0; offset < queue.count; ++offset) {
    queue.values[offset] = rebuilt[offset];
  }
  ++midiFramesReplaced;
  return true;
}

static bool auxVoiceHasControlValueHead(uint8_t voiceIndex) {
  if (!voiceHasPending(voiceIndex)) {
    return false;
  }
  const uint8_t value = frontVoiceValue(voiceIndex);
  if (voiceIndex == VOICE_DMC) {
    return isDmcControlValueByte(value);
  }
  return isVoiceControlValueByte(value);
}

static bool auxVoiceHasControlMetaHead(uint8_t voiceIndex) {
  if (!voiceHasPending(voiceIndex)) {
    return false;
  }
  const uint8_t value = frontVoiceValue(voiceIndex);
  if (voiceIndex == VOICE_DMC) {
    return isDmcControlMetaByte(value);
  }
  return isVoiceControlMetaByte(value);
}

static void selectAuxSnapshot(uint8_t* auxVoice, uint8_t* auxValue) {
  const bool triPending = voiceHasPending(VOICE_TRI);
  const bool noisePending = voiceHasPending(VOICE_NOISE);
  const bool dmcPending = voiceHasPending(VOICE_DMC);
  uint8_t bestVoice = VOICE_NONE;
  uint8_t bestScore = 0;
  uint8_t voiceIndex = slotScanStart;

  *auxVoice = VOICE_NONE;
  *auxValue = 0;

  if (!triPending && !noisePending && !dmcPending) {
    if (selectPendingControl(auxValue)) {
      *auxVoice = VOICE_CTRL;
    }
    return;
  }

  if (!dmcPending) {
    dmcAuxDeferrals = 0;
  }
  if (auxVoiceHasControlValueHead(VOICE_TRI)) {
    *auxVoice = VOICE_TRI;
    *auxValue = frontVoiceValue(VOICE_TRI);
    if (dmcPending && dmcAuxDeferrals < 0xFFU) ++dmcAuxDeferrals;
    return;
  }
  if (auxVoiceHasControlValueHead(VOICE_NOISE)) {
    *auxVoice = VOICE_NOISE;
    *auxValue = frontVoiceValue(VOICE_NOISE);
    if (dmcPending && dmcAuxDeferrals < 0xFFU) ++dmcAuxDeferrals;
    return;
  }
  if (auxVoiceHasControlMetaHead(VOICE_NOISE)) {
    *auxVoice = VOICE_NOISE;
    *auxValue = frontVoiceValue(VOICE_NOISE);
    if (dmcPending && dmcAuxDeferrals < 0xFFU) ++dmcAuxDeferrals;
    return;
  }
  if (auxVoiceHasControlMetaHead(VOICE_TRI)) {
    *auxVoice = VOICE_TRI;
    *auxValue = frontVoiceValue(VOICE_TRI);
    if (dmcPending && dmcAuxDeferrals < 0xFFU) ++dmcAuxDeferrals;
    return;
  }

  if (dmcPending && (!triPending && !noisePending)) {
    *auxVoice = VOICE_DMC;
    *auxValue = frontVoiceValue(VOICE_DMC);
    dmcAuxDeferrals = 0;
    return;
  }

  if (dmcPending && dmcAuxDeferrals >= DMC_AUX_DEFERRAL_LIMIT) {
    *auxVoice = VOICE_DMC;
    *auxValue = frontVoiceValue(VOICE_DMC);
    dmcAuxDeferrals = 0;
    return;
  }

  for (uint8_t i = 0; i < 3U; ++i) {
    if (!auxVoiceSelectable(voiceIndex) ||
        !voiceHasPending(voiceIndex)) {
      voiceIndex = nextAuxVoice(voiceIndex);
      continue;
    }
    const uint8_t score = auxVoicePriorityScore(voiceIndex);
    if (bestVoice == VOICE_NONE || score > bestScore) {
      bestVoice = voiceIndex;
      bestScore = score;
    }
    voiceIndex = nextAuxVoice(voiceIndex);
  }

  if (bestVoice != VOICE_NONE) {
    *auxVoice = bestVoice;
    *auxValue = frontVoiceValue(bestVoice);
    if (bestVoice == VOICE_DMC) {
      dmcAuxDeferrals = 0;
    } else if (dmcPending && dmcAuxDeferrals < 0xFFU) {
      ++dmcAuxDeferrals;
    }
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
  if (voiceHasPending(VOICE_DMC)) {
    frontVoiceEvent.regId = REG_DMC_NOTE;
    frontVoiceEvent.value = frontVoiceValue(VOICE_DMC);
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
  const uint8_t dmcPending = pendingCountForVoice(VOICE_DMC);
  const uint8_t ctrlPending = controlPendingCount();

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
  Serial.print(F(" | C="));
  Serial.print(ctrlPending);
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
  Serial.print(F(" | DMC="));
  if (!voiceHasPending(VOICE_DMC)) {
    Serial.print(F("--"));
  } else {
    const uint8_t value = frontVoiceValue(VOICE_DMC);
    if (value < 16U) {
      Serial.print('0');
    }
    Serial.print(value, HEX);
  }
  Serial.print('/');
  Serial.print(dmcPending);
}

static void printPanelTarget(uint8_t target) {
  switch (target) {
    case VOICE_P1: Serial.print(F("P1")); break;
    case VOICE_P2: Serial.print(F("P2")); break;
    case VOICE_TRI: Serial.print(F("TRI")); break;
    case VOICE_NOISE: Serial.print(F("NOI")); break;
    case PANEL_TARGET_GLOBAL: Serial.print(F("GLB")); break;
    default: Serial.print(F("--")); break;
  }
}

static void printPanelLfoTarget(uint8_t target) {
  switch (target) {
    case PANEL_LFO_TARGET_PITCH: Serial.print(F("PIT")); break;
    case PANEL_LFO_TARGET_DUTY: Serial.print(F("DUT")); break;
    case PANEL_LFO_TARGET_AMP: Serial.print(F("AMP")); break;
    default: Serial.print(F("--")); break;
  }
}

static void printPanelChannel(uint8_t channel) {
  switch (channel) {
    case MCP_CH_VOLUME: Serial.print('V'); break;
    case MCP_CH_ATTACK: Serial.print('A'); break;
    case MCP_CH_DECAY: Serial.print('D'); break;
    case MCP_CH_RELEASE: Serial.print('R'); break;
    case MCP_CH_LFO_DEPTH: Serial.print(F("LD")); break;
    case MCP_CH_LFO_RATE: Serial.print(F("LR")); break;
    case MCP_CH_DUTY: Serial.print(F("DU")); break;
    case MCP_CH_ARP_TIME: Serial.print(panelLastAppliedArpTimeWasDelay ? F("DL") : F("AR")); break;
    default: Serial.print(F("--")); break;
  }
}

static void printMidiState() {
  uint8_t arpMask = 0U;
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
  Serial.print(F(" | MC="));
  Serial.print(midiControlReceived);
  Serial.print('/');
  Serial.print(midiControlApplied);
  Serial.print(F(" | PC="));
  Serial.print(panelControlApplied);
  Serial.print(F(" | CS="));
  Serial.print(scheduledControlPendingTotal());
  Serial.print('/');
  Serial.print(controlScheduleInjectedPairs);
  Serial.print(F(" | DD="));
  Serial.print(dmcDroppedTriggers);
  Serial.print(F(" | NC="));
  Serial.print(noteCohortActive ? noteCohortPendingCount() : 0U);
  Serial.print('/');
  Serial.print(noteCohortFlushes);
  Serial.print(F(" | PT="));
  printPanelTarget(panelLastTarget);
  Serial.print(F(" | VM="));
  Serial.print(panelLastVoiceMask, HEX);
  Serial.print(F(" | PL="));
  printPanelLfoTarget(panelLastLfoTarget);
  Serial.print(F(" | LM="));
  Serial.print(panelLastLfoMask, HEX);
  Serial.print(F(" | PV="));
  for (uint8_t channel = 0; channel < PANEL_ANALOG_CHANNELS; ++channel) {
    if (panelCurrentQuant[channel] == 0xFFU) {
      Serial.print('-');
    } else {
      Serial.print(panelCurrentQuant[channel], HEX);
    }
  }
  if (ENABLE_PANEL_RAW_LOG) {
    Serial.print(F(" | PR="));
    for (uint8_t channel = 0; channel < PANEL_ANALOG_CHANNELS; ++channel) {
      if (channel != 0U) Serial.print(',');
      if (panelFiltered[channel] < 0x100U) Serial.print('0');
      if (panelFiltered[channel] < 0x010U) Serial.print('0');
      Serial.print(panelFiltered[channel], HEX);
    }
  }
  Serial.print(F(" | PA="));
  printPanelChannel(panelLastAppliedChannel);
  Serial.print(':');
  if (panelLastAppliedValue == 0xFFU) {
    Serial.print('-');
  } else {
    Serial.print(panelLastAppliedValue, HEX);
  }
  Serial.print(F(" | PS="));
  Serial.print(panelAnalogSuppressed);
  if (arpStates[VOICE_P1].enabled) arpMask |= 0x01U;
  if (arpStates[VOICE_P2].enabled) arpMask |= 0x02U;
  if (arpStates[VOICE_TRI].enabled) arpMask |= 0x04U;
  if (arpStates[VOICE_NOISE].enabled) arpMask |= 0x08U;
  Serial.print(F(" | CLK="));
  Serial.print(midiClockRunning ? F("ON") : F("OFF"));
  Serial.print(F(" | AT="));
  Serial.print(midiClockTicks);
  Serial.print(F(" | ARP="));
  Serial.print(arpMask, HEX);
  Serial.print(F(" | AS="));
  if (arpStates[VOICE_P1].enabled && midiVoices[VOICE_P1].stackCount != 0U) Serial.print(arpStates[VOICE_P1].stepIndex); else Serial.print('-');
  Serial.print(',');
  if (arpStates[VOICE_P2].enabled && midiVoices[VOICE_P2].stackCount != 0U) Serial.print(arpStates[VOICE_P2].stepIndex); else Serial.print('-');
  Serial.print(',');
  if (arpStates[VOICE_TRI].enabled && midiVoices[VOICE_TRI].stackCount != 0U) Serial.print(arpStates[VOICE_TRI].stepIndex); else Serial.print('-');
  Serial.print(',');
  if (arpStates[VOICE_NOISE].enabled && midiVoices[VOICE_NOISE].stackCount != 0U) Serial.print(arpStates[VOICE_NOISE].stepIndex); else Serial.print('-');
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
  uint8_t pattern = STATUS_IDLE_PATTERN;
  if (latchedP1Pending) pattern |= STATUS_BIT_P1_PENDING;
  if (latchedP2Pending) pattern |= STATUS_BIT_P2_PENDING;
  if (latchedAuxPending) {
    pattern |= STATUS_BIT_AUX_PENDING;
    pattern |= latchedAuxStatusType();
  }
  return pattern;
}

static uint8_t latchAndBuildStatusPattern() {
  latchTransportSnapshot();
  return buildStatusPattern();
}

static uint8_t buildP1LoPattern() {
  return latchedP1Pending ? (uint8_t)(latchedP1Value & 0x1FU)
                          : STATUS_IDLE_PATTERN;
}

static uint8_t buildP1HiPattern() {
  return latchedP1Pending ? (uint8_t)((latchedP1Value >> 5) & 0x07U)
                          : STATUS_IDLE_PATTERN;
}

static uint8_t buildP2LoPattern() {
  return latchedP2Pending ? (uint8_t)(latchedP2Value & 0x1FU)
                          : STATUS_IDLE_PATTERN;
}

static uint8_t buildP2HiPattern() {
  return latchedP2Pending ? (uint8_t)((latchedP2Value >> 5) & 0x07U)
                          : STATUS_IDLE_PATTERN;
}

static uint8_t buildAuxLoPattern() {
  if (!latchedAuxPending) return STATUS_IDLE_PATTERN;
  if (!latchedAuxUsesFullByte() && latchedAuxVoice == VOICE_NOISE) {
    return (uint8_t)((latchedAuxValue & 0x0FU) |
                     ((latchedAuxValue & COMPACT_GATE_BIT) ? 0x10U : 0x00U));
  }
  return (uint8_t)(latchedAuxValue & 0x1FU);
}

static uint8_t buildAuxHiPattern() {
  if (!latchedAuxPending) return STATUS_IDLE_PATTERN;
  return (uint8_t)((latchedAuxValue >> 5) & 0x07U);
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
  controlDirtyMask = 0;
  controlScanStart = 0;
  controlScheduleVoiceCursor = 0;
  controlScheduleControlCursor = 0;
  lastControlScheduleInjectMs = 0;
  noteCohortActive = false;
  noteCohortStartUs = 0;
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    voiceQueues[voiceIndex].head = 0;
    voiceQueues[voiceIndex].tail = 0;
    voiceQueues[voiceIndex].count = 0;
    preAckVoiceValid[voiceIndex] = false;
    preAckVoiceValue[voiceIndex] = 0;
    noteCohortPending[voiceIndex] = false;
    noteCohortValue[voiceIndex] = 0;
    for (uint8_t controlIndex = 0; controlIndex < CTRL_SLOT_COUNT; ++controlIndex) {
      controlSchedulePending[voiceIndex][controlIndex] = false;
      controlScheduleValue[voiceIndex][controlIndex] = 0;
    }
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
  if (latchedAuxPending) {
    if (ackVoiceEvent(latchedAuxVoice)) {
      acked = true;
      slotScanStart = nextAuxVoice(latchedAuxVoice);
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
  if (channel == CH_DMC) return VOICE_NONE;
  if (channel == CH_P1) return VOICE_P1;
  if (channel == CH_P2) return VOICE_P2;
  if (channel == CH_TRI) return VOICE_TRI;
  if (channel == CH_NOISE) return VOICE_NOISE;
  return VOICE_NONE;
}

static uint8_t regForVoice(uint8_t voiceIndex) {
  return (voiceIndex == VOICE_P1) ? REG_P1_NOTE :
         (voiceIndex == VOICE_P2) ? REG_P2_NOTE :
         (voiceIndex == VOICE_TRI) ? REG_TRI_NOTE :
         (voiceIndex == VOICE_NOISE) ? REG_NOI_NOTE :
         REG_DMC_NOTE;
}

static uint8_t mapDmcSampleId(uint8_t midiNote) {
  if (midiNote >= 36U && midiNote < (uint8_t)(36U + DMC_SAMPLE_COUNT)) {
    return (uint8_t)(midiNote - 36U);
  }
  return 0xFFU;
}

static uint8_t sustainLevelFromMidi(uint8_t value) {
  if (value < 8U) {
    return 0U;
  }
  const uint32_t x = (uint32_t)(value - 8U);
  const uint32_t maxValue = 119UL * 119UL;
  uint8_t level = (uint8_t)((x * x * 15UL + (maxValue / 2UL)) / maxValue);
  if (level > 15U) level = 15U;
  return level;
}

static inline uint8_t nibbleFromCc(uint8_t value) {
  return (uint8_t)(value >> 3);
}

static inline uint8_t dutyFromCc(uint8_t value) {
  uint8_t duty = (uint8_t)(value >> 5);
  if (duty > 2U) {
    duty = 2U;
  }
  return duty;
}

static inline uint8_t lfoWaveFromCc(uint8_t value) {
  return (uint8_t)((value >> 5) & 0x03U);
}

static inline uint8_t dmcPitchFromCc(uint8_t value) {
  uint8_t mapped = (uint8_t)(value >> 3);
  if (mapped > 15U) mapped = 15U;
  return mapped;
}

static bool midiCcDisabledForPanelDebug(uint8_t control) {
  return control == CC_ATTACK ||
         control == CC_DECAY ||
         control == CC_VOLUME ||
         control == CC_RELEASE ||
         control == CC_DUTY ||
         control == CC_LFO_DEPTH ||
         control == CC_LFO_RATE ||
         control == CC_ARP_ENABLE ||
         control == CC_ARP_DIV ||
         control == CC_LFO_DUTY_DEPTH ||
         control == CC_LFO_AMP_DEPTH ||
         control == CC_LFO_DELAY;
}

static uint8_t voiceModeFlags(uint8_t voiceIndex) {
  uint8_t flags = 0U;
  if (voiceSupportsArp(voiceIndex) && arpStates[voiceIndex].enabled) {
    flags |= 0x01U;
  }
  return flags;
}

static void syncVoiceModeControl(uint8_t voiceIndex) {
  if (!voiceSupportsArp(voiceIndex)) {
    return;
  }
  enqueueVoiceControlChange(voiceIndex,
                            CTRL_MODE_FLAGS,
                            voiceModeFlags(voiceIndex),
                            "MIDI voice mode");
}

static bool setArpEnabledForVoice(uint8_t voiceIndex, bool enabled) {
  if (!voiceSupportsArp(voiceIndex)) {
    return false;
  }
  if (arpStates[voiceIndex].enabled == enabled) {
    return false;
  }
  arpStates[voiceIndex].enabled = enabled;
  syncVoiceModeControl(voiceIndex);
  resetArpTransport(voiceIndex, enabled && !midiClockRunning);
  return true;
}

static bool setArpDivisionForVoice(uint8_t voiceIndex, uint8_t divisionIndex) {
  if (!voiceSupportsArp(voiceIndex)) {
    return false;
  }
  if (arpStates[voiceIndex].divisionIndex == divisionIndex) {
    return false;
  }
  arpStates[voiceIndex].divisionIndex = divisionIndex;
  arpStates[voiceIndex].clockCounter = 0U;
  return true;
}

static inline bool panelPinActive(uint8_t pin) {
  const bool isHigh = (digitalRead(pin) == HIGH);
  return PANEL_SWITCH_ACTIVE_LOW ? !isHigh : isHigh;
}

static void setupPanelInputs() {
  pinMode(PIN_PANEL_LFO_PITCH, INPUT_PULLUP);
  pinMode(PIN_PANEL_LFO_DUTY, INPUT_PULLUP);
  pinMode(PIN_PANEL_LFO_AMP, INPUT_PULLUP);
  pinMode(PIN_PANEL_VOICE_P1, INPUT_PULLUP);
  pinMode(PIN_PANEL_VOICE_P2, INPUT_PULLUP);
  pinMode(PIN_PANEL_VOICE_TRI, INPUT_PULLUP);
  pinMode(PIN_PANEL_VOICE_NOISE, INPUT_PULLUP);
  pinMode(PIN_PANEL_VOICE_GLOBAL, INPUT_PULLUP);
  pinMode(PIN_PANEL_ARP_ENABLE, INPUT_PULLUP);

  pinMode(PIN_MCP_CS, OUTPUT);
  digitalWrite(PIN_MCP_CS, HIGH);
  SPI.setRX(PIN_MCP_MISO);
  SPI.setTX(PIN_MCP_MOSI);
  SPI.setSCK(PIN_MCP_SCK);
  SPI.begin();
  panelSpiReady = true;
}

static uint16_t readMcp3008(uint8_t channel) {
  if (!panelSpiReady) {
    return 0U;
  }

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

static uint8_t panelVoiceTargetMask() {
  uint8_t mask = 0U;
  if (panelPinActive(PIN_PANEL_VOICE_P1)) mask |= 0x01U;
  if (panelPinActive(PIN_PANEL_VOICE_P2)) mask |= 0x02U;
  if (panelPinActive(PIN_PANEL_VOICE_TRI)) mask |= 0x04U;
  if (panelPinActive(PIN_PANEL_VOICE_NOISE)) mask |= 0x08U;
  if (panelPinActive(PIN_PANEL_VOICE_GLOBAL)) mask |= 0x10U;
  return mask;
}

static uint8_t panelLfoTargetMaskActiveLow() {
  uint8_t mask = 0U;
  if (panelPinActive(PIN_PANEL_LFO_PITCH)) mask |= 0x01U;
  if (panelPinActive(PIN_PANEL_LFO_DUTY)) mask |= 0x02U;
  if (panelPinActive(PIN_PANEL_LFO_AMP)) mask |= 0x04U;
  return mask;
}

static bool panelMaskIsSingleTarget(uint8_t mask) {
  return mask != 0U && (mask & (uint8_t)(mask - 1U)) == 0U;
}

static uint8_t panelLfoTargetMask() {
  // Keep the LFO selector on one stable polarity. Flipping pull-up/pull-down
  // every scan made the mechanical selector appear latched after one move.
  pinMode(PIN_PANEL_LFO_PITCH, INPUT_PULLUP);
  pinMode(PIN_PANEL_LFO_DUTY, INPUT_PULLUP);
  pinMode(PIN_PANEL_LFO_AMP, INPUT_PULLUP);
  delayMicroseconds(5);
  const uint8_t activeLow = panelLfoTargetMaskActiveLow();
  if (panelMaskIsSingleTarget(activeLow)) {
    return activeLow;
  }
  return 0x01U;
}

static uint8_t panelSelectedVoiceTarget() {
  panelLastVoiceMask = panelVoiceTargetMask();
  switch (panelLastVoiceMask) {
    case 0x01U: return VOICE_P1;
    case 0x02U: return VOICE_P2;
    case 0x04U: return VOICE_TRI;
    case 0x08U: return VOICE_NOISE;
    case 0x10U: return PANEL_TARGET_GLOBAL;
    default: return VOICE_NONE;
  }
}

static uint8_t panelSelectedLfoTarget() {
  panelLastLfoMask = panelLfoTargetMask();
  switch (panelLastLfoMask) {
    case 0x01U: return PANEL_LFO_TARGET_PITCH;
    case 0x02U: return PANEL_LFO_TARGET_DUTY;
    case 0x04U: return PANEL_LFO_TARGET_AMP;
    default: return PANEL_LFO_TARGET_NONE;
  }
}

static uint8_t panelAnalogToNibble(uint16_t raw) {
  if (raw > 1023U) raw = 1023U;
  return (uint8_t)((raw * 15U + 511U) / 1023U);
}

static uint8_t panelAttackFromRaw(uint16_t raw) {
  if (raw > 1023U) raw = 1023U;
  if (raw < 848U) return 0U;
  if (raw < 928U) return 1U;
  if (raw < 984U) return 2U;
  return 3U;
}

static uint8_t panelReleaseFromRaw(uint16_t raw) {
  if (raw > 1023U) raw = 1023U;
  if (raw < 96U) return 0U;
  raw = (uint16_t)(raw - 96U);
  return (uint8_t)((raw * 15U + 463U) / 927U);
}

static uint8_t panelPulseDutyFromRaw(uint16_t raw) {
  if (raw < 341U) return 0U;
  if (raw < 682U) return 1U;
  return 2U;
}

static uint8_t panelArpDivisionFromNibble(uint8_t value) {
  if (value < 4U) return 0U;
  if (value < 8U) return 1U;
  if (value < 12U) return 2U;
  return 3U;
}

static void panelPrimeAnalogLatch(const uint16_t* raw, const uint8_t* quant) {
  for (uint8_t channel = 0; channel < PANEL_ANALOG_CHANNELS; ++channel) {
    panelLastRaw[channel] = raw[channel];
    panelLastQuant[channel] = quant[channel];
  }
}

static bool panelChannelEnabled(uint8_t channel) {
  return channel < PANEL_ANALOG_CHANNELS;
}

static uint16_t panelRawDeadband(uint8_t channel) {
  switch (channel) {
    case MCP_CH_ATTACK:
    case MCP_CH_DECAY:
    case MCP_CH_RELEASE:
      return PANEL_ADSR_RAW_DEADBAND;
    case MCP_CH_LFO_DEPTH:
    case MCP_CH_LFO_RATE:
      return PANEL_LFO_RAW_DEADBAND;
    default:
      return PANEL_DEFAULT_RAW_DEADBAND;
  }
}

static bool panelChannelCanBreakLock(uint8_t channel, uint16_t delta) {
  return delta >= (uint16_t)(panelRawDeadband(channel) * 4U);
}

static uint8_t panelArpTimeEffectiveValue(uint8_t quant, bool panelArpEnabled) {
  return panelArpEnabled ? panelArpDivisionFromNibble(quant) : (uint8_t)(quant & 0x0FU);
}

static bool panelArpTimeCandidateStable(uint8_t effectiveValue, bool isDelay, uint32_t nowMs) {
  if (panelArpTimeCandidateValue != effectiveValue ||
      panelArpTimeCandidateIsDelay != isDelay) {
    panelArpTimeCandidateValue = effectiveValue;
    panelArpTimeCandidateIsDelay = isDelay;
    panelArpTimeCandidateSinceMs = nowMs;
    return false;
  }
  return (uint32_t)(nowMs - panelArpTimeCandidateSinceMs) >= PANEL_ARP_TIME_STABLE_MS;
}

static bool panelQuantWouldChange(uint8_t channel, uint16_t raw, uint8_t quant, uint16_t* deltaOut) {
  uint16_t previousRaw;
  uint16_t delta;

  if (deltaOut != nullptr) {
    *deltaOut = 0U;
  }
  if (!panelChannelEnabled(channel)) {
    return false;
  }

  if (panelLastQuant[channel] == 0xFFU) {
    return false;
  }

  if (panelLastQuant[channel] == quant) {
    panelLastRaw[channel] = raw;
    return false;
  }

  previousRaw = panelLastRaw[channel];
  delta = (raw > previousRaw) ? (uint16_t)(raw - previousRaw) : (uint16_t)(previousRaw - raw);
  if (delta < panelRawDeadband(channel)) {
    return false;
  }

  if (deltaOut != nullptr) {
    *deltaOut = delta;
  }
  return true;
}

static bool panelQuantWouldChangeForMode(uint8_t channel,
                                         uint16_t raw,
                                         uint8_t quant,
                                         bool panelArpEnabled,
                                         uint32_t nowMs,
                                         uint16_t* deltaOut) {
  if (channel == MCP_CH_ARP_TIME && panelLastQuant[channel] != 0xFFU) {
    const uint8_t effective = panelArpTimeEffectiveValue(quant, panelArpEnabled);
    const uint8_t previous = panelArpTimeEffectiveValue(panelLastQuant[channel], panelArpEnabled);
    if (effective == previous) {
      panelLastRaw[channel] = raw;
      if (deltaOut != nullptr) {
        *deltaOut = 0U;
      }
      return false;
    }
    if (!panelArpTimeCandidateStable(effective, !panelArpEnabled, nowMs)) {
      if (deltaOut != nullptr) {
        *deltaOut = 0U;
      }
      return false;
    }
  }
  return panelQuantWouldChange(channel, raw, quant, deltaOut);
}

static void panelCommitAnalogLatch(uint8_t channel, uint16_t raw, uint8_t quant) {
  if (channel >= PANEL_ANALOG_CHANNELS) {
    return;
  }
  panelLastQuant[channel] = quant;
  panelLastRaw[channel] = raw;
}

static uint8_t panelNormalizeControlValue(uint8_t voiceIndex, uint8_t controlIndex, uint8_t value) {
  if (controlIndex == CTRL_DUTY) {
    return (uint8_t)(value & ((voiceIndex == VOICE_NOISE) ? 0x0FU : 0x03U));
  }
  if (controlIndex == CTRL_LFO_WAVE) {
    return (uint8_t)(value & 0x03U);
  }
  if (controlIndex == CTRL_VOLUME && value == 0U) {
    return 1U;
  }
  return (uint8_t)(value & 0x0FU);
}

static bool panelApplyVoiceControl(uint8_t voiceIndex, uint8_t controlIndex, uint8_t value, const char* reason) {
  uint8_t* target;
  uint8_t normalized;

  if (!voiceSupportsControl(voiceIndex, controlIndex)) {
    return false;
  }
  target = voiceControlField(voiceIndex, controlIndex);
  if (target == nullptr) {
    return false;
  }
  normalized = panelNormalizeControlValue(voiceIndex, controlIndex, value);
  if (*target == normalized) {
    return false;
  }
  if (enqueueVoiceControlChange(voiceIndex, controlIndex, normalized, reason)) {
    ++panelControlApplied;
    return true;
  }
  return false;
}

static bool panelApplyTargetControl(uint8_t target, uint8_t controlIndex, uint8_t value, const char* reason) {
  bool changed = false;

  if (target == VOICE_NONE) {
    return false;
  }

  if (target == PANEL_TARGET_GLOBAL) {
    if (panelApplyVoiceControl(VOICE_NOISE, controlIndex, value, reason)) changed = true;
    if (panelApplyVoiceControl(VOICE_P1, controlIndex, value, reason)) changed = true;
    if (panelApplyVoiceControl(VOICE_P2, controlIndex, value, reason)) changed = true;
    return changed;
  }

  return panelApplyVoiceControl(target, controlIndex, value, reason);
}

static bool panelTargetSupportsControl(uint8_t target, uint8_t controlIndex) {
  if (target == VOICE_NONE) {
    return false;
  }
  if (target == PANEL_TARGET_GLOBAL) {
    return voiceSupportsControl(VOICE_NOISE, controlIndex) ||
           voiceSupportsControl(VOICE_P1, controlIndex) ||
           voiceSupportsControl(VOICE_P2, controlIndex);
  }
  return voiceSupportsControl(target, controlIndex);
}

static uint8_t panelControlForLfoTarget(uint8_t lfoTarget) {
  if (lfoTarget == PANEL_LFO_TARGET_DUTY) {
    return CTRL_LFO_DUTY_DEPTH;
  }
  if (lfoTarget == PANEL_LFO_TARGET_AMP) {
    return CTRL_LFO_AMP_DEPTH;
  }
  if (lfoTarget == PANEL_LFO_TARGET_PITCH) {
    return CTRL_LFO_DEPTH;
  }
  return 0xFFU;
}

static bool panelApplyLfoDepth(uint8_t target, uint8_t lfoTarget, uint8_t value) {
  const uint8_t selectedControl = panelControlForLfoTarget(lfoTarget);
  bool changed = false;

  if (selectedControl == 0xFFU || !panelTargetSupportsControl(target, selectedControl)) {
    return false;
  }

  // The physical selector is exclusive: moving from Pitch to Duty or Amp must
  // clear the other supported destinations, otherwise old depths keep sounding mixed.
  if (panelApplyTargetControl(target, CTRL_LFO_DEPTH,
                              (selectedControl == CTRL_LFO_DEPTH) ? value : 0U,
                              "panel lfo exclusive")) changed = true;
  if (panelApplyTargetControl(target, CTRL_LFO_DUTY_DEPTH,
                              (selectedControl == CTRL_LFO_DUTY_DEPTH) ? value : 0U,
                              "panel lfo exclusive")) changed = true;
  if (panelApplyTargetControl(target, CTRL_LFO_AMP_DEPTH,
                              (selectedControl == CTRL_LFO_AMP_DEPTH) ? value : 0U,
                              "panel lfo exclusive")) changed = true;
  return changed;
}

static bool panelApplyCurrentLfoControls(uint8_t target,
                                         uint8_t lfoTarget,
                                         uint8_t depthValue,
                                         uint8_t rateValue,
                                         uint8_t delayValue,
                                         bool panelArpEnabled) {
  const uint8_t selectedControl = panelControlForLfoTarget(lfoTarget);
  bool changed = false;
  (void)panelArpEnabled;
  if (selectedControl == 0xFFU || !panelTargetSupportsControl(target, selectedControl)) {
    return false;
  }
  if (panelApplyLfoDepth(target, lfoTarget, depthValue)) changed = true;
  if (panelApplyTargetControl(target, CTRL_LFO_RATE, rateValue, "panel lfo rate sync")) changed = true;
  if (panelApplyTargetControl(target, CTRL_LFO_DELAY, delayValue, "panel lfo delay sync")) changed = true;
  return changed;
}

static bool panelApplyArpEnabled(uint8_t target, bool enabled) {
  bool changed = false;
  if (target == VOICE_NONE) {
    return false;
  }
  if (target == PANEL_TARGET_GLOBAL) {
    for (uint8_t voiceIndex = VOICE_P1; voiceIndex <= VOICE_TRI; ++voiceIndex) {
      if (setArpEnabledForVoice(voiceIndex, enabled)) changed = true;
    }
  } else if (setArpEnabledForVoice(target, enabled)) {
    changed = true;
  }
  if (changed) {
    ++panelControlApplied;
  }
  return changed;
}

static bool panelApplyArpDivision(uint8_t target, uint8_t divisionIndex) {
  bool changed = false;
  if (target == VOICE_NONE) {
    return false;
  }
  if (target == PANEL_TARGET_GLOBAL) {
    for (uint8_t voiceIndex = VOICE_P1; voiceIndex <= VOICE_TRI; ++voiceIndex) {
      if (setArpDivisionForVoice(voiceIndex, divisionIndex)) changed = true;
    }
  } else if (setArpDivisionForVoice(target, divisionIndex)) {
    changed = true;
  }
  if (changed) {
    ++panelControlApplied;
  }
  return changed;
}

static bool panelApplyAnalogChannel(uint8_t channel, uint8_t target, uint8_t lfoTarget, uint8_t value, bool panelArpEnabled) {
  switch (channel) {
    case MCP_CH_VOLUME:
      return panelApplyTargetControl(target, CTRL_VOLUME, value, "panel volume");
    case MCP_CH_ATTACK:
      return panelApplyTargetControl(target, CTRL_ATTACK, value, "panel attack");
    case MCP_CH_DECAY:
      return panelApplyTargetControl(target, CTRL_DECAY, value, "panel decay");
    case MCP_CH_RELEASE:
      return panelApplyTargetControl(target, CTRL_RELEASE, value, "panel release");
    case MCP_CH_LFO_DEPTH:
      return panelApplyLfoDepth(target, lfoTarget, value);
    case MCP_CH_LFO_RATE:
      return panelApplyTargetControl(target, CTRL_LFO_RATE, value, "panel lfo rate");
    case MCP_CH_DUTY:
      return panelApplyTargetControl(target, CTRL_DUTY, value, "panel duty/timbre");
    case MCP_CH_ARP_TIME:
      if (panelArpEnabled) {
        return panelApplyArpDivision(target, panelArpDivisionFromNibble(value));
      }
      return panelApplyTargetControl(target, CTRL_LFO_DELAY, value, "panel lfo delay");
    default:
      return false;
  }
}

static void servicePanelInputs(uint32_t nowMs) {
  uint16_t raw[PANEL_ANALOG_CHANNELS];
  uint8_t quant[PANEL_ANALOG_CHANNELS];
  bool selectorAcceptedChanged = false;
  uint8_t previousAcceptedTarget = panelLastTarget;
  uint8_t previousAcceptedLfoTarget = panelLastLfoTarget;
  const bool firstAnalogScan = !panelAnalogPrimed;

  if (!panelSpiReady || (uint32_t)(nowMs - lastPanelScanMs) < PANEL_SCAN_INTERVAL_MS) {
    return;
  }
  lastPanelScanMs = nowMs;

  const uint8_t rawTarget = panelSelectedVoiceTarget();
  const uint8_t rawLfoTarget = panelSelectedLfoTarget();
  if (rawTarget != panelCandidateTarget || rawLfoTarget != panelCandidateLfoTarget) {
    panelCandidateTarget = rawTarget;
    panelCandidateLfoTarget = rawLfoTarget;
    panelSelectorStableCount = 1U;
  } else if (panelSelectorStableCount < PANEL_SELECTOR_STABLE_SCANS) {
    ++panelSelectorStableCount;
  }

  if (panelSelectorStableCount >= PANEL_SELECTOR_STABLE_SCANS &&
      (panelCandidateTarget != panelLastTarget || panelCandidateLfoTarget != panelLastLfoTarget)) {
    previousAcceptedTarget = panelLastTarget;
    previousAcceptedLfoTarget = panelLastLfoTarget;
    panelLastTarget = panelCandidateTarget;
    panelLastLfoTarget = panelCandidateLfoTarget;
    selectorAcceptedChanged = true;
  }

  const uint8_t target = panelLastTarget;
  const uint8_t lfoTarget = panelLastLfoTarget;
  const bool panelArpEnabled = panelPinActive(PIN_PANEL_ARP_ENABLE);
  const bool panelArpSwitchChanged = panelArpEnabled != panelLastArpSwitch;

  for (uint8_t channel = 0; channel < PANEL_ANALOG_CHANNELS; ++channel) {
    const uint16_t sample = readMcp3008(channel);
    if (firstAnalogScan) {
      panelFiltered[channel] = sample;
    } else {
      panelFiltered[channel] = (uint16_t)((panelFiltered[channel] + sample + 1U) >> 1);
    }
    raw[channel] = panelFiltered[channel];
  }

  quant[MCP_CH_VOLUME] = panelAnalogToNibble(raw[MCP_CH_VOLUME]);
  quant[MCP_CH_ATTACK] = panelAttackFromRaw(raw[MCP_CH_ATTACK]);
  quant[MCP_CH_DECAY] = panelAnalogToNibble(raw[MCP_CH_DECAY]);
  quant[MCP_CH_RELEASE] = panelReleaseFromRaw(raw[MCP_CH_RELEASE]);
  quant[MCP_CH_LFO_DEPTH] = panelAnalogToNibble(raw[MCP_CH_LFO_DEPTH]);
  quant[MCP_CH_LFO_RATE] = panelAnalogToNibble(raw[MCP_CH_LFO_RATE]);
  quant[MCP_CH_DUTY] = (target == VOICE_NOISE || target == PANEL_TARGET_GLOBAL)
    ? panelAnalogToNibble(raw[MCP_CH_DUTY])
    : panelPulseDutyFromRaw(raw[MCP_CH_DUTY]);
  quant[MCP_CH_ARP_TIME] = panelAnalogToNibble(raw[MCP_CH_ARP_TIME]);

  for (uint8_t channel = 0; channel < PANEL_ANALOG_CHANNELS; ++channel) {
    panelCurrentQuant[channel] = quant[channel];
  }

  if (firstAnalogScan || selectorAcceptedChanged) {
    if (!firstAnalogScan && selectorAcceptedChanged &&
        (target != previousAcceptedTarget || lfoTarget != previousAcceptedLfoTarget)) {
      panelApplyCurrentLfoControls(target,
                                   lfoTarget,
                                   quant[MCP_CH_LFO_DEPTH],
                                   quant[MCP_CH_LFO_RATE],
                                   quant[MCP_CH_ARP_TIME],
                                   panelArpEnabled);
    }
    if (!firstAnalogScan && panelArpSwitchChanged) {
      panelApplyArpEnabled(target, panelArpEnabled);
      if (!panelArpEnabled) {
        panelApplyCurrentLfoControls(target,
                                     lfoTarget,
                                     quant[MCP_CH_LFO_DEPTH],
                                     quant[MCP_CH_LFO_RATE],
                                     quant[MCP_CH_ARP_TIME],
                                     false);
      }
      if (panelApplyAnalogChannel(MCP_CH_ARP_TIME, target, lfoTarget, quant[MCP_CH_ARP_TIME], panelArpEnabled)) {
        panelLastAppliedChannel = MCP_CH_ARP_TIME;
        panelLastAppliedValue = quant[MCP_CH_ARP_TIME];
        panelLastAppliedArpTimeWasDelay = !panelArpEnabled;
      }
    }
    panelLastArpSwitch = panelArpEnabled;
    panelPrimeAnalogLatch(raw, quant);
    panelAnalogPrimed = true;
    panelActiveAnalogChannel = 0xFFU;
    panelActiveAnalogUntilMs = 0;
    return;
  }

  if (panelArpSwitchChanged) {
    panelApplyArpEnabled(target, panelArpEnabled);
    if (!panelArpEnabled) {
      panelApplyCurrentLfoControls(target,
                                   lfoTarget,
                                   quant[MCP_CH_LFO_DEPTH],
                                   quant[MCP_CH_LFO_RATE],
                                   quant[MCP_CH_ARP_TIME],
                                   false);
    }
    if (panelApplyAnalogChannel(MCP_CH_ARP_TIME, target, lfoTarget, quant[MCP_CH_ARP_TIME], panelArpEnabled)) {
      panelLastAppliedChannel = MCP_CH_ARP_TIME;
      panelLastAppliedValue = quant[MCP_CH_ARP_TIME];
      panelLastAppliedArpTimeWasDelay = !panelArpEnabled;
    }
    panelLastArpSwitch = panelArpEnabled;
  }

  uint8_t bestChannel = 0xFFU;
  uint16_t bestDelta = 0U;
  uint8_t changedCount = 0U;
  const bool lockActive = panelActiveAnalogChannel != 0xFFU &&
                          (int32_t)(panelActiveAnalogUntilMs - nowMs) > 0;
  if (!lockActive) {
    panelActiveAnalogChannel = 0xFFU;
  }
  for (uint8_t channel = 0; channel < PANEL_ANALOG_CHANNELS; ++channel) {
    uint16_t delta = 0U;
    if (panelQuantWouldChangeForMode(channel, raw[channel], quant[channel], panelArpEnabled, nowMs, &delta)) {
      ++changedCount;
      if (lockActive && channel == panelActiveAnalogChannel) {
        bestDelta = delta;
        bestChannel = channel;
        continue;
      }
      if (lockActive && !panelChannelCanBreakLock(channel, delta)) {
        continue;
      }
      if (bestChannel == 0xFFU || delta > bestDelta) {
        bestDelta = delta;
        bestChannel = channel;
      }
    }
  }

  if (bestChannel != 0xFFU) {
    const bool applied = panelApplyAnalogChannel(bestChannel, target, lfoTarget, quant[bestChannel], panelArpEnabled);
    if (applied) {
      panelLastAppliedChannel = bestChannel;
      panelLastAppliedValue = quant[bestChannel];
      panelLastAppliedArpTimeWasDelay = (bestChannel == MCP_CH_ARP_TIME && !panelArpEnabled);
      panelActiveAnalogChannel = bestChannel;
      panelActiveAnalogUntilMs = nowMs + PANEL_ACTIVE_LOCK_MS;
    }
    panelCommitAnalogLatch(bestChannel, raw[bestChannel], quant[bestChannel]);

    if (lockActive) {
      for (uint8_t channel = 0; channel < PANEL_ANALOG_CHANNELS; ++channel) {
        uint16_t delta = 0U;
        if (channel != bestChannel &&
            panelQuantWouldChangeForMode(channel, raw[channel], quant[channel], panelArpEnabled, nowMs, &delta) &&
            !panelChannelCanBreakLock(channel, delta)) {
          panelCommitAnalogLatch(channel, raw[channel], quant[channel]);
        }
      }
    }

    if (changedCount > 1U) {
      panelAnalogSuppressed = (uint32_t)(panelAnalogSuppressed + changedCount - 1U);
    }
  } else if (changedCount != 0U) {
    if (lockActive) {
      for (uint8_t channel = 0; channel < PANEL_ANALOG_CHANNELS; ++channel) {
        uint16_t delta = 0U;
        if (panelQuantWouldChangeForMode(channel, raw[channel], quant[channel], panelArpEnabled, nowMs, &delta) &&
            !panelChannelCanBreakLock(channel, delta)) {
          panelCommitAnalogLatch(channel, raw[channel], quant[channel]);
        }
      }
    }
    panelAnalogSuppressed = (uint32_t)(panelAnalogSuppressed + changedCount);
  }

  panelApplyArpEnabled(target, panelArpEnabled);
}

static bool queueControlChange(uint8_t controlIndex, uint8_t value, const char* reason) {
  uint8_t* target = nullptr;
  const bool wasPending = controlPending(controlIndex);

  switch (controlIndex) {
    case CTRL_ATTACK: target = &currentControlState.attack; value &= 0x0FU; break;
    case CTRL_DECAY: target = &currentControlState.decay; value &= 0x0FU; break;
    case CTRL_VOLUME: target = &currentControlState.sustain; value &= 0x0FU; break;
    case CTRL_RELEASE: target = &currentControlState.release; value &= 0x0FU; break;
    case CTRL_DUTY: target = &currentControlState.duty; value &= 0x03U; break;
    case CTRL_LFO_DEPTH: target = &currentControlState.lfoDepth; value &= 0x0FU; break;
    case CTRL_LFO_RATE: target = &currentControlState.lfoRate; value &= 0x0FU; break;
    default: return false;
  }

  if (*target == value && !controlPending(controlIndex)) {
    return true;
  }

  *target = value;
  controlDirtyMask |= controlMaskBit(controlIndex);
  if (wasPending) {
    ++midiFramesReplaced;
  } else {
    ++midiFramesEnqueued;
  }
  ++midiControlApplied;
  touchQueueHighWater();
  lastMidiEventMs = millis();
  logMidiPressure(reason);
  return true;
}

static bool enqueueVoiceControlChange(uint8_t voiceIndex, uint8_t controlIndex, uint8_t value, const char* reason) {
  uint8_t* target;

  if (!voiceSupportsControl(voiceIndex, controlIndex)) {
    return false;
  }

  target = voiceControlField(voiceIndex, controlIndex);
  if (target == nullptr) {
    return false;
  }

  switch (controlIndex) {
    case CTRL_ATTACK:
    case CTRL_DECAY:
    case CTRL_VOLUME:
    case CTRL_RELEASE:
    case CTRL_LFO_DEPTH:
    case CTRL_LFO_RATE:
    case CTRL_LFO_DUTY_DEPTH:
    case CTRL_LFO_AMP_DEPTH:
    case CTRL_LFO_DELAY:
    case CTRL_LFO_WAVE:
    case CTRL_MODE_FLAGS:
      value &= 0x0FU;
      break;
    case CTRL_DUTY:
      value &= (voiceIndex == VOICE_NOISE) ? 0x0FU : 0x03U;
      break;
    default:
      return false;
  }

  if (*target == value && !controlSchedulePending[voiceIndex][controlIndex]) {
    return true;
  }

  *target = value;
  controlScheduleValue[voiceIndex][controlIndex] = value;
  if (controlSchedulePending[voiceIndex][controlIndex]) {
    ++midiFramesReplaced;
  } else {
    controlSchedulePending[voiceIndex][controlIndex] = true;
    ++controlSchedulePendingWrites;
  }
  ++midiControlApplied;
  lastMidiEventMs = millis();
  (void)reason;
  return true;
}

static uint8_t scheduledControlPendingTotal() {
  uint8_t total = 0U;
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    for (uint8_t controlIndex = 0; controlIndex < CTRL_SLOT_COUNT; ++controlIndex) {
      if (controlSchedulePending[voiceIndex][controlIndex]) {
        ++total;
      }
    }
  }
  return total;
}

static uint8_t controlPriorityAt(uint8_t offset) {
  static const uint8_t priority[CTRL_SLOT_COUNT] = {
    CTRL_MODE_FLAGS,
    CTRL_RELEASE,
    CTRL_DUTY,
    CTRL_LFO_DUTY_DEPTH,
    CTRL_LFO_AMP_DEPTH,
    CTRL_LFO_DEPTH,
    CTRL_LFO_RATE,
    CTRL_LFO_DELAY,
    CTRL_ATTACK,
    CTRL_DECAY,
    CTRL_VOLUME,
    CTRL_LFO_WAVE,
    CTRL_DMC_TRIGGER
  };
  return priority[offset % CTRL_SLOT_COUNT];
}

static uint8_t controlVoicePriorityAt(uint8_t offset) {
  static const uint8_t priority[VOICE_COUNT] = {
    VOICE_NOISE,
    VOICE_P1,
    VOICE_P2,
    VOICE_TRI,
    VOICE_DMC
  };
  return priority[offset % VOICE_COUNT];
}

static bool schedulerCanInjectControl(uint8_t voiceIndex) {
  if (!transportPrimed || voiceIndex >= VOICE_COUNT) {
    return false;
  }
  if (voiceIndex == VOICE_NOISE && voiceHasPending(VOICE_TRI)) {
    return false;
  }
  if (voiceIndex == VOICE_TRI && voiceHasPending(VOICE_NOISE)) {
    return false;
  }
  return true;
}

static bool injectScheduledControl(uint8_t voiceIndex, uint8_t controlIndex) {
  VoiceEdgeQueue& queue = voiceQueues[voiceIndex];
  const uint8_t capacity = (uint8_t)(sizeof(queue.values) / sizeof(queue.values[0]));
  uint8_t value;
  uint8_t meta;
  uint8_t payload;

  if (!controlSchedulePending[voiceIndex][controlIndex]) {
    return false;
  }
  if (!voiceSupportsControl(voiceIndex, controlIndex)) {
    controlSchedulePending[voiceIndex][controlIndex] = false;
    return false;
  }
  if (!schedulerCanInjectControl(voiceIndex) || queue.count > (uint8_t)(capacity - 2U)) {
    return false;
  }

  value = controlScheduleValue[voiceIndex][controlIndex];
  meta = voiceControlMetaByte(voiceIndex, controlIndex);
  payload = voiceControlValueByte(voiceIndex, value);
  if (!enqueueVoiceEvent(voiceIndex, meta) || !enqueueVoiceEvent(voiceIndex, payload)) {
    overflowLatched = true;
    return false;
  }

  controlSchedulePending[voiceIndex][controlIndex] = false;
  midiFramesEnqueued = (uint32_t)(midiFramesEnqueued + 2U);
  ++controlScheduleInjectedPairs;
  lastMidiEventMs = millis();
  return true;
}

static void serviceControlScheduler() {
  const uint32_t nowMs = millis();
  if (scheduledControlPendingTotal() == 0U) {
    return;
  }
  if (totalPendingCount() > PANEL_CONTROL_RELEASE_Q) {
    return;
  }
  if (lastControlScheduleInjectMs != 0U &&
      (uint32_t)(nowMs - lastControlScheduleInjectMs) < PANEL_CONTROL_INJECT_INTERVAL_MS) {
    return;
  }

  for (uint8_t voiceOffset = 0; voiceOffset < VOICE_COUNT; ++voiceOffset) {
    const uint8_t voiceIndex = controlVoicePriorityAt((uint8_t)(controlScheduleVoiceCursor + voiceOffset));
    if (!schedulerCanInjectControl(voiceIndex)) {
      continue;
    }
    for (uint8_t controlOffset = 0; controlOffset < CTRL_SLOT_COUNT; ++controlOffset) {
      const uint8_t controlIndex = controlPriorityAt((uint8_t)(controlScheduleControlCursor + controlOffset));
      if (injectScheduledControl(voiceIndex, controlIndex)) {
        lastControlScheduleInjectMs = nowMs;
        controlScheduleVoiceCursor = (uint8_t)((controlScheduleVoiceCursor + voiceOffset + 1U) % VOICE_COUNT);
        controlScheduleControlCursor = (uint8_t)((controlScheduleControlCursor + controlOffset + 1U) % CTRL_SLOT_COUNT);
        return;
      }
    }
  }
}

static void handleMidiControlChange(uint8_t channel, uint8_t control, uint8_t value) {
  uint8_t voiceIndex = VOICE_NONE;

  if (channel != CH_GLOBAL &&
      channel != CH_P1 &&
      channel != CH_P2 &&
      channel != CH_TRI &&
      channel != CH_NOISE) {
    return;
  }

  ++midiControlReceived;
  flushNoteCohort();

  if (midiCcDisabledForPanelDebug(control)) {
    return;
  }

  switch (channel) {
    case CH_P1: voiceIndex = VOICE_P1; break;
    case CH_P2: voiceIndex = VOICE_P2; break;
    case CH_TRI: voiceIndex = VOICE_TRI; break;
    case CH_NOISE: voiceIndex = VOICE_NOISE; break;
    default: break;
  }

  switch (control) {
    case CC_ATTACK:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_ATTACK, nibbleFromCc(value), "MIDI cc attack");
        enqueueVoiceControlChange(VOICE_P1, CTRL_ATTACK, nibbleFromCc(value), "MIDI cc attack");
        enqueueVoiceControlChange(VOICE_P2, CTRL_ATTACK, nibbleFromCc(value), "MIDI cc attack");
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_ATTACK, nibbleFromCc(value), "MIDI cc attack");
      }
      break;
    case CC_DECAY:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_DECAY, nibbleFromCc(value), "MIDI cc decay");
        enqueueVoiceControlChange(VOICE_P1, CTRL_DECAY, nibbleFromCc(value), "MIDI cc decay");
        enqueueVoiceControlChange(VOICE_P2, CTRL_DECAY, nibbleFromCc(value), "MIDI cc decay");
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_DECAY, nibbleFromCc(value), "MIDI cc decay");
      }
      break;
    case CC_VOLUME:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_VOLUME, nibbleFromCc(value), "MIDI cc volume");
        enqueueVoiceControlChange(VOICE_P1, CTRL_VOLUME, nibbleFromCc(value), "MIDI cc volume");
        enqueueVoiceControlChange(VOICE_P2, CTRL_VOLUME, nibbleFromCc(value), "MIDI cc volume");
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_VOLUME, nibbleFromCc(value), "MIDI cc volume");
      }
      break;
    case CC_RELEASE:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_RELEASE, nibbleFromCc(value), "MIDI cc release");
        enqueueVoiceControlChange(VOICE_P1, CTRL_RELEASE, nibbleFromCc(value), "MIDI cc release");
        enqueueVoiceControlChange(VOICE_P2, CTRL_RELEASE, nibbleFromCc(value), "MIDI cc release");
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_RELEASE, nibbleFromCc(value), "MIDI cc release");
      }
      break;
    case CC_DUTY:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_DUTY, nibbleFromCc(value), "MIDI cc noise timbre");
        enqueueVoiceControlChange(VOICE_P1, CTRL_DUTY, dutyFromCc(value), "MIDI cc duty");
        enqueueVoiceControlChange(VOICE_P2, CTRL_DUTY, dutyFromCc(value), "MIDI cc duty");
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex,
                                  CTRL_DUTY,
                                  (voiceIndex == VOICE_NOISE) ? nibbleFromCc(value) : dutyFromCc(value),
                                  (voiceIndex == VOICE_NOISE) ? "MIDI cc noise timbre" : "MIDI cc duty");
      }
      break;
    case CC_LFO_DEPTH:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_P1, CTRL_LFO_DEPTH, nibbleFromCc(value), "MIDI cc lfo depth");
        enqueueVoiceControlChange(VOICE_P2, CTRL_LFO_DEPTH, nibbleFromCc(value), "MIDI cc lfo depth");
        /* LFO disabled on Triangle to reduce system load */
        /* enqueueVoiceControlChange(VOICE_TRI, CTRL_LFO_DEPTH, nibbleFromCc(value), "MIDI cc lfo depth"); */
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_LFO_DEPTH, nibbleFromCc(value), "MIDI cc lfo depth");
      }
      break;
    case CC_LFO_RATE:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_LFO_RATE, nibbleFromCc(value), "MIDI cc lfo rate");
        enqueueVoiceControlChange(VOICE_P1, CTRL_LFO_RATE, nibbleFromCc(value), "MIDI cc lfo rate");
        enqueueVoiceControlChange(VOICE_P2, CTRL_LFO_RATE, nibbleFromCc(value), "MIDI cc lfo rate");
        /* LFO disabled on Triangle to reduce system load */
        /* enqueueVoiceControlChange(VOICE_TRI, CTRL_LFO_RATE, nibbleFromCc(value), "MIDI cc lfo rate"); */
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_LFO_RATE, nibbleFromCc(value), "MIDI cc lfo rate");
      }
      break;
    case CC_LFO_DUTY_DEPTH:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_P1, CTRL_LFO_DUTY_DEPTH, nibbleFromCc(value), "MIDI cc duty lfo");
        enqueueVoiceControlChange(VOICE_P2, CTRL_LFO_DUTY_DEPTH, nibbleFromCc(value), "MIDI cc duty lfo");
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_LFO_DUTY_DEPTH, nibbleFromCc(value), "MIDI cc duty lfo");
      }
      break;
    case CC_LFO_AMP_DEPTH:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_LFO_AMP_DEPTH, nibbleFromCc(value), "MIDI cc amp lfo");
        enqueueVoiceControlChange(VOICE_P1, CTRL_LFO_AMP_DEPTH, nibbleFromCc(value), "MIDI cc amp lfo");
        enqueueVoiceControlChange(VOICE_P2, CTRL_LFO_AMP_DEPTH, nibbleFromCc(value), "MIDI cc amp lfo");
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_LFO_AMP_DEPTH, nibbleFromCc(value), "MIDI cc amp lfo");
      }
      break;
    case CC_LFO_DELAY:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_LFO_DELAY, nibbleFromCc(value), "MIDI cc lfo delay");
        enqueueVoiceControlChange(VOICE_P1, CTRL_LFO_DELAY, nibbleFromCc(value), "MIDI cc lfo delay");
        enqueueVoiceControlChange(VOICE_P2, CTRL_LFO_DELAY, nibbleFromCc(value), "MIDI cc lfo delay");
        /* LFO disabled on Triangle to reduce system load */
        /* enqueueVoiceControlChange(VOICE_TRI, CTRL_LFO_DELAY, nibbleFromCc(value), "MIDI cc lfo delay"); */
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_LFO_DELAY, nibbleFromCc(value), "MIDI cc lfo delay");
      }
      break;
    case CC_LFO_WAVE:
      if (channel == CH_GLOBAL) {
        enqueueVoiceControlChange(VOICE_NOISE, CTRL_LFO_WAVE, lfoWaveFromCc(value), "MIDI cc lfo wave");
        enqueueVoiceControlChange(VOICE_P1, CTRL_LFO_WAVE, lfoWaveFromCc(value), "MIDI cc lfo wave");
        enqueueVoiceControlChange(VOICE_P2, CTRL_LFO_WAVE, lfoWaveFromCc(value), "MIDI cc lfo wave");
        /* LFO disabled on Triangle to reduce system load */
        /* enqueueVoiceControlChange(VOICE_TRI, CTRL_LFO_WAVE, lfoWaveFromCc(value), "MIDI cc lfo wave"); */
      } else if (voiceIndex != VOICE_NONE) {
        enqueueVoiceControlChange(voiceIndex, CTRL_LFO_WAVE, lfoWaveFromCc(value), "MIDI cc lfo wave");
      }
      break;
    case CC_DMC_PITCH:
      break;
    case CC_ARP_ENABLE:
      {
        bool changed = false;
        const bool enabled = value >= 64U;
      if (channel == CH_GLOBAL) {
        for (uint8_t targetVoice = VOICE_P1; targetVoice <= VOICE_NOISE; ++targetVoice) {
          if (setArpEnabledForVoice(targetVoice, enabled)) {
            changed = true;
          }
        }
      } else if (voiceIndex != VOICE_NONE && voiceSupportsArp(voiceIndex)) {
        changed = setArpEnabledForVoice(voiceIndex, enabled);
      }
      if (changed) {
        ++midiControlApplied;
      }
      break;
      }
    case CC_ARP_DIV:
      {
        bool changed = false;
        const uint8_t divisionIndex = arpDivisionFromCc(value);
      if (channel == CH_GLOBAL) {
        for (uint8_t targetVoice = VOICE_P1; targetVoice <= VOICE_NOISE; ++targetVoice) {
          if (setArpDivisionForVoice(targetVoice, divisionIndex)) {
            changed = true;
          }
        }
      } else if (voiceIndex != VOICE_NONE && voiceSupportsArp(voiceIndex)) {
        changed = setArpDivisionForVoice(voiceIndex, divisionIndex);
      }
      if (changed) {
        ++midiControlApplied;
      }
      break;
      }
    default:
      break;
  }
}

static void handleMidiPitchBend(uint8_t channel, uint8_t lsb, uint8_t msb) {
  (void)channel;
  (void)lsb;
  (void)msb;
}

static bool enqueueDmcTrigger(uint8_t sampleId, const char* reason) {
  VoiceEdgeQueue& queue = voiceQueues[VOICE_DMC];
  const uint8_t capacity = (uint8_t)(sizeof(queue.values) / sizeof(queue.values[0]));

  if (sampleId >= DMC_SAMPLE_COUNT) {
    return false;
  }
  if (anyArpActive() && musicalPendingCount() >= DMC_ARP_PRESSURE_DROP_Q) {
    ++dmcDroppedTriggers;
    return false;
  }
  if (pendingCountForVoice(VOICE_DMC) >= DMC_MAX_PENDING_EVENTS) {
    ++dmcDroppedTriggers;
    return false;
  }

  if (!transportPrimed) {
    if (!enqueueVoiceEvent(VOICE_DMC, sampleId)) {
      overflowLatched = true;
      logMidiPressure(reason);
      return false;
    }
    ++midiFramesEnqueued;
    ++midiPreAckLatched;
    lastMidiEventMs = millis();
    return true;
  }

  if (queue.count >= capacity) {
    overflowLatched = true;
    logMidiPressure(reason);
    return false;
  }

  if (!enqueueVoiceEvent(VOICE_DMC, sampleId)) {
    overflowLatched = true;
    logMidiPressure(reason);
    return false;
  }

  ++midiFramesEnqueued;
  lastMidiEventMs = millis();
  logMidiPressure(reason);
  return true;
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
  uint8_t insertAt;

  if (stackContains(voice, midiNote)) {
    return true;
  }
  if (voice.stackCount >= (sizeof(voice.stack) / sizeof(voice.stack[0]))) {
    for (uint8_t i = 1; i < voice.stackCount; ++i) {
      voice.stack[i - 1U] = voice.stack[i];
    }
    --voice.stackCount;
  }

  insertAt = voice.stackCount;
  for (uint8_t i = 0; i < voice.stackCount; ++i) {
    if (midiNote < voice.stack[i]) {
      insertAt = i;
      break;
    }
  }
  for (uint8_t i = voice.stackCount; i > insertAt; --i) {
    voice.stack[i] = voice.stack[i - 1U];
  }
  voice.stack[insertAt] = midiNote;
  ++voice.stackCount;
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

static bool replacePendingCompactVoice(uint8_t voiceIndex, uint8_t compactValue) {
  VoiceEdgeQueue& queue = voiceQueues[voiceIndex];

  if (queue.count != 0U) {
    uint8_t index = queue.tail;
    for (uint8_t remaining = queue.count; remaining != 0U; --remaining) {
      index = (uint8_t)((index + (sizeof(queue.values) / sizeof(queue.values[0])) - 1U) %
                        (sizeof(queue.values) / sizeof(queue.values[0])));
      if ((queue.values[index] & COMPACT_GATE_BIT) != 0U) {
        queue.values[index] = compactValue;
        ++midiFramesReplaced;
        return true;
      }
      if ((queue.values[index] & 0xF0U) != VOICE_CTRL_META_BASE &&
          (queue.values[index] & 0xF0U) != VOICE_CTRL_VALUE_BASE &&
          !isDmcControlMetaByte(queue.values[index]) &&
          !isDmcControlValueByte(queue.values[index])) {
        break;
      }
    }
  }

  if (preAckVoiceValid[voiceIndex] && (preAckVoiceValue[voiceIndex] & COMPACT_GATE_BIT) != 0U) {
    preAckVoiceValue[voiceIndex] = compactValue;
    ++midiFramesReplaced;
    return true;
  }

  return false;
}

static bool isCompactVoiceStateByte(uint8_t value) {
  return !isVoiceControlMetaByte(value) &&
         !isVoiceControlValueByte(value) &&
         !isDmcControlMetaByte(value) &&
         !isDmcControlValueByte(value);
}

static bool replacePendingCompactVoiceState(uint8_t voiceIndex, uint8_t compactValue) {
  VoiceEdgeQueue& queue = voiceQueues[voiceIndex];
  const uint8_t capacity = (uint8_t)(sizeof(queue.values) / sizeof(queue.values[0]));

  if (queue.count != 0U) {
    uint8_t index = queue.tail;
    for (uint8_t remaining = queue.count; remaining != 0U; --remaining) {
      index = (uint8_t)((index + capacity - 1U) % capacity);
      if (isCompactVoiceStateByte(queue.values[index])) {
        queue.values[index] = compactValue;
        ++midiFramesReplaced;
        return true;
      }
    }
  }

  if (preAckVoiceValid[voiceIndex] && isCompactVoiceStateByte(preAckVoiceValue[voiceIndex])) {
    preAckVoiceValue[voiceIndex] = compactValue;
    ++midiFramesReplaced;
    return true;
  }

  return false;
}

static inline bool voiceSupportsArp(uint8_t voiceIndex) {
  return voiceIndex == VOICE_P1 ||
         voiceIndex == VOICE_P2 ||
         voiceIndex == VOICE_TRI;
}

static inline bool voiceSupportsAutoGlide(uint8_t voiceIndex) {
  return voiceIndex == VOICE_P1;
}

static inline uint8_t arpStepClocks(uint8_t divisionIndex) {
  return kArpClocksPerStep[(divisionIndex < 4U) ? divisionIndex : 2U];
}

static uint8_t arpDivisionFromCc(uint8_t value) {
  if (value < 32U) return 0U;
  if (value < 64U) return 1U;
  if (value < 96U) return 2U;
  return 3U;
}

static uint8_t arpCurrentMidiNote(const MidiVoice& voice, const ArpState& arp) {
  if (voice.stackCount == 0U) {
    return 0U;
  }
  return voice.stack[arp.stepIndex % voice.stackCount];
}

static bool triggerVoiceStackNote(uint8_t voiceIndex, const char* reason);
static bool enqueueArpVoiceState(uint8_t voiceIndex, uint8_t nesNote, bool gate, const char* reason);
static bool enqueueCompactVoice(uint8_t voiceIndex, uint8_t nesNote, bool gate, bool allowReplace, const char* reason);

static bool triggerVoiceStackNote(uint8_t voiceIndex, const char* reason) {
  MidiVoice& voice = midiVoices[voiceIndex];
  if (voice.stackCount == 0U) {
    return false;
  }

  const uint8_t midiNote = voiceSupportsArp(voiceIndex) && arpStates[voiceIndex].enabled
    ? arpCurrentMidiNote(voice, arpStates[voiceIndex])
    : topVoiceNote(voice);
  const uint8_t nesNote = mapVoiceNote(voiceIndex, midiNote);

  if (voiceSupportsArp(voiceIndex) && arpStates[voiceIndex].enabled) {
    if (!enqueueArpVoiceState(voiceIndex, nesNote, true, reason)) {
      return false;
    }
  } else {
    if (!enqueueCompactVoice(voiceIndex, nesNote, true, false, reason)) {
      return false;
    }
  }

  voice.active = true;
  voice.midiNote = midiNote;
  voice.nesNote = nesNote;
  return true;
}

static void advanceArpStepIndex(uint8_t voiceIndex) {
  MidiVoice& voice = midiVoices[voiceIndex];
  if (voice.stackCount <= 1U) {
    arpStates[voiceIndex].stepIndex = 0U;
    return;
  }
  arpStates[voiceIndex].stepIndex = (uint8_t)((arpStates[voiceIndex].stepIndex + 1U) % voice.stackCount);
}

static void resetArpTransport(uint8_t voiceIndex, bool retriggerCurrent) {
  if (!voiceSupportsArp(voiceIndex)) {
    return;
  }

  ArpState& arp = arpStates[voiceIndex];
  MidiVoice& voice = midiVoices[voiceIndex];
  arp.clockCounter = 0U;
  if (voice.stackCount == 0U) {
    arp.stepIndex = 0U;
    return;
  }

  arp.stepIndex = 0U;
  if (retriggerCurrent) {
    triggerVoiceStackNote(voiceIndex, "ARP retrig");
    advanceArpStepIndex(voiceIndex);
  } else if (midiClockRunning) {
    const uint8_t stepClocks = arpStepClocks(arp.divisionIndex);
    arp.clockCounter = (stepClocks > 0U) ? (uint8_t)(stepClocks - 1U) : 0U;
  }
}

static void advanceArpClock() {
  if (!midiClockRunning) {
    return;
  }

  ++midiClockTicks;
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    if (!voiceSupportsArp(voiceIndex)) {
      continue;
    }
    ArpState& arp = arpStates[voiceIndex];
    MidiVoice& voice = midiVoices[voiceIndex];
    if (!arp.enabled || voice.stackCount == 0U) {
      continue;
    }

    ++arp.clockCounter;
    if (arp.clockCounter < arpStepClocks(arp.divisionIndex)) {
      continue;
    }

    triggerVoiceStackNote(voiceIndex, "ARP tick");
    arp.clockCounter = 0U;
    advanceArpStepIndex(voiceIndex);
  }
}

static void resetAllArpClocks(bool resetStep) {
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    arpStates[voiceIndex].clockCounter = 0U;
    if (resetStep) {
      arpStates[voiceIndex].stepIndex = 0U;
    }
  }
}

static bool enqueueCompactVoiceImmediate(uint8_t voiceIndex, uint8_t nesNote, bool gate, bool allowReplace, const char* reason) {
  uint8_t compactValue = (uint8_t)(nesNote & 0x7FU);

  if (gate) {
    compactValue |= COMPACT_GATE_BIT;
  }

  if (!transportPrimed) {
    if (allowReplace && gate && replacePendingCompactVoice(voiceIndex, compactValue)) {
      lastMidiEventMs = millis();
      return true;
    }
    latchPreAckVoice(voiceIndex, compactValue);
    ++midiFramesEnqueued;
    ++midiPreAckLatched;
    lastMidiEventMs = millis();
    return true;
  }

  if (allowReplace && gate && replacePendingCompactVoice(voiceIndex, compactValue)) {
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

static bool enqueueArpVoiceState(uint8_t voiceIndex, uint8_t nesNote, bool gate, const char* reason) {
  uint8_t compactValue = (uint8_t)(nesNote & 0x7FU);

  if (gate) {
    compactValue |= COMPACT_GATE_BIT;
  }

  if (replacePendingCompactVoiceState(voiceIndex, compactValue)) {
    lastMidiEventMs = millis();
    return true;
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

static bool enqueueCompactVoice(uint8_t voiceIndex, uint8_t nesNote, bool gate, bool allowReplace, const char* reason) {
  return enqueueCompactVoiceImmediate(voiceIndex, nesNote, gate, allowReplace, reason);
}

static uint8_t noteCohortPendingCount() {
  uint8_t count = 0U;
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    if (noteCohortPending[voiceIndex]) {
      ++count;
    }
  }
  return count;
}

static bool flushNoteCohort() {
  bool ok = true;
  if (!noteCohortActive) {
    return true;
  }
  for (uint8_t voiceIndex = VOICE_P1; voiceIndex <= VOICE_NOISE; ++voiceIndex) {
    if (!noteCohortPending[voiceIndex]) {
      continue;
    }
    const uint8_t value = noteCohortValue[voiceIndex];
    const bool gate = (value & COMPACT_GATE_BIT) != 0U;
    const uint8_t nesNote = (uint8_t)(value & 0x7FU);
    if (!enqueueCompactVoiceImmediate(voiceIndex, nesNote, gate, true, "MIDI note cohort")) {
      ok = false;
    }
    noteCohortPending[voiceIndex] = false;
    noteCohortValue[voiceIndex] = 0;
  }
  noteCohortActive = false;
  noteCohortStartUs = 0;
  ++noteCohortFlushes;
  return ok;
}

static void serviceNoteCohort(uint32_t nowUs) {
  if (noteCohortActive && (uint32_t)(nowUs - noteCohortStartUs) >= NOTE_COHORT_WINDOW_US) {
    flushNoteCohort();
  }
}

static bool enqueueCompactVoiceCohort(uint8_t voiceIndex, uint8_t nesNote, bool gate) {
  flushNoteCohort();
  return enqueueCompactVoiceImmediate(voiceIndex, nesNote, gate, true, "MIDI note direct");
}

static void handleMidiNoteOff(uint8_t channel, uint8_t midiNote);
static void handleMidiPitchBend(uint8_t channel, uint8_t lsb, uint8_t msb);

static void handleMidiNoteOn(uint8_t channel, uint8_t midiNote, uint8_t velocity) {
  uint8_t voiceIndex;
  MidiVoice* voice;
  uint8_t topMidiNote;
  uint8_t nesNote;

  if (velocity == 0) {
    handleMidiNoteOff(channel, midiNote);
    return;
  }

  if (channel == CH_DMC) {
    const uint8_t sampleId = mapDmcSampleId(midiNote);
    if (sampleId != 0xFFU) {
      enqueueDmcTrigger(sampleId, "MIDI dmc trig");
    }
    return;
  }

  voiceIndex = voiceFromChannel(channel);
  if (voiceIndex == VOICE_NONE) return;
  voice = &midiVoices[voiceIndex];
  pushVoiceNote(*voice, midiNote);
  topMidiNote = topVoiceNote(*voice);
  if (topMidiNote == 0U) return;

  if (voiceSupportsArp(voiceIndex) && arpStates[voiceIndex].enabled) {
    resetArpTransport(voiceIndex, !midiClockRunning);
    return;
  }

  if (voice->active && voice->midiNote == topMidiNote) {
    voice->nesNote = mapVoiceNote(voiceIndex, topMidiNote);
    return;
  }

  nesNote = mapVoiceNote(voiceIndex, topMidiNote);
  if (enqueueCompactVoiceCohort(voiceIndex, nesNote, true)) {
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

  if (channel == CH_DMC) return;

  flushNoteCohort();

  voiceIndex = voiceFromChannel(channel);
  if (voiceIndex == VOICE_NONE) return;
  voice = &midiVoices[voiceIndex];
  if (!removeVoiceNote(*voice, midiNote)) {
    ++midiIgnoredNoteOffs;
    return;
  }

  if (voiceSupportsArp(voiceIndex) && arpStates[voiceIndex].enabled) {
    if (voice->stackCount != 0U) {
      arpStates[voiceIndex].stepIndex %= voice->stackCount;
      if (!midiClockRunning) {
        triggerVoiceStackNote(voiceIndex, "ARP resume");
        advanceArpStepIndex(voiceIndex);
      }
      return;
    }
    arpStates[voiceIndex].clockCounter = 0U;
    if (!voice->active) {
      return;
    }
    if (enqueueArpVoiceState(voiceIndex, voice->nesNote, false, "ARP note off")) {
      voice->active = false;
    }
    return;
  }

  if (voice->stackCount != 0U) {
    resumeMidiNote = topVoiceNote(*voice);
    resumeNesNote = mapVoiceNote(voiceIndex, resumeMidiNote);
    if (enqueueCompactVoice(voiceIndex, resumeNesNote, true, true, "MIDI note resume")) {
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

  if (enqueueCompactVoice(voiceIndex, voice->nesNote, false, false, "MIDI note off")) {
    voice->active = false;
  }
}

static void handleAllNotesOff(uint8_t channel) {
  const bool clearP1 = (channel == CH_P1 || channel == CH_GLOBAL);
  const bool clearP2 = (channel == CH_P2 || channel == CH_GLOBAL);
  const bool clearTri = (channel == CH_TRI || channel == CH_GLOBAL);
  const bool clearNoise = (channel == CH_NOISE || channel == CH_GLOBAL);

  if (!clearP1 && !clearP2 && !clearTri && !clearNoise) return;
  flushNoteCohort();

  if (clearP1 && midiVoices[VOICE_P1].active) {
    clearVoiceStack(midiVoices[VOICE_P1]);
    arpStates[VOICE_P1].clockCounter = 0U;
    arpStates[VOICE_P1].stepIndex = 0U;
    if ((arpStates[VOICE_P1].enabled
        ? enqueueArpVoiceState(VOICE_P1, midiVoices[VOICE_P1].nesNote, false, "MIDI all off")
        : enqueueCompactVoice(VOICE_P1, midiVoices[VOICE_P1].nesNote, false, false, "MIDI all off"))) {
      midiVoices[VOICE_P1].active = false;
    }
  }
  if (clearP2 && midiVoices[VOICE_P2].active) {
    clearVoiceStack(midiVoices[VOICE_P2]);
    arpStates[VOICE_P2].clockCounter = 0U;
    arpStates[VOICE_P2].stepIndex = 0U;
    if ((arpStates[VOICE_P2].enabled
        ? enqueueArpVoiceState(VOICE_P2, midiVoices[VOICE_P2].nesNote, false, "MIDI all off")
        : enqueueCompactVoice(VOICE_P2, midiVoices[VOICE_P2].nesNote, false, false, "MIDI all off"))) {
      midiVoices[VOICE_P2].active = false;
    }
  }
  if (clearTri && midiVoices[VOICE_TRI].active) {
    clearVoiceStack(midiVoices[VOICE_TRI]);
    arpStates[VOICE_TRI].clockCounter = 0U;
    arpStates[VOICE_TRI].stepIndex = 0U;
    if ((arpStates[VOICE_TRI].enabled
        ? enqueueArpVoiceState(VOICE_TRI, midiVoices[VOICE_TRI].nesNote, false, "MIDI all off")
        : enqueueCompactVoice(VOICE_TRI, midiVoices[VOICE_TRI].nesNote, false, false, "MIDI all off"))) {
      midiVoices[VOICE_TRI].active = false;
    }
  }
  if (clearNoise && midiVoices[VOICE_NOISE].active) {
    clearVoiceStack(midiVoices[VOICE_NOISE]);
    arpStates[VOICE_NOISE].clockCounter = 0U;
    arpStates[VOICE_NOISE].stepIndex = 0U;
    if (enqueueCompactVoice(VOICE_NOISE, midiVoices[VOICE_NOISE].nesNote, false, false, "MIDI all off")) {
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
  } else if (type == 0xB0U) {
    if (data1 == 120U || data1 == 123U) {
      handleAllNotesOff(channel);
    } else {
      handleMidiControlChange(channel, data1, data2);
    }
  } else if (type == 0xE0U) {
    handleMidiPitchBend(channel, data1, data2);
  }
}

static void stopArpPlayback() {
  for (uint8_t voiceIndex = VOICE_P1; voiceIndex <= VOICE_NOISE; ++voiceIndex) {
    if (!voiceSupportsArp(voiceIndex) || !arpStates[voiceIndex].enabled) {
      continue;
    }
    if (midiVoices[voiceIndex].active) {
      enqueueArpVoiceState(voiceIndex, midiVoices[voiceIndex].nesNote, false, "MIDI stop arp off");
    }
    clearVoiceStack(midiVoices[voiceIndex]);
    midiVoices[voiceIndex].active = false;
    midiVoices[voiceIndex].midiNote = 0U;
    midiVoices[voiceIndex].nesNote = 0U;
    arpStates[voiceIndex].clockCounter = 0U;
    arpStates[voiceIndex].stepIndex = 0U;
  }
}

static void handleMidiRealtime(uint8_t status) {
  switch (status) {
    case 0xF8U:
      flushNoteCohort();
      advanceArpClock();
      break;
    case 0xFAU:
      flushNoteCohort();
      midiClockRunning = true;
      resetAllArpClocks(true);
      for (uint8_t voiceIndex = VOICE_P1; voiceIndex <= VOICE_NOISE; ++voiceIndex) {
        if (voiceSupportsArp(voiceIndex) && arpStates[voiceIndex].enabled) {
          syncVoiceModeControl(voiceIndex);
        }
      }
      break;
    case 0xFBU:
      flushNoteCohort();
      midiClockRunning = true;
      resetAllArpClocks(false);
      for (uint8_t voiceIndex = VOICE_P1; voiceIndex <= VOICE_NOISE; ++voiceIndex) {
        if (voiceSupportsArp(voiceIndex) && arpStates[voiceIndex].enabled) {
          syncVoiceModeControl(voiceIndex);
        }
      }
      break;
    case 0xFCU:
      flushNoteCohort();
      midiClockRunning = false;
      resetAllArpClocks(false);
      stopArpPlayback();
      break;
    default:
      break;
  }
}

static void handleMidiByte(uint8_t b) {
  if (b & 0x80U) {
    if (b >= 0xF8U) {
      handleMidiRealtime(b);
      return;
    }
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
    voiceControls[voiceIndex] = kInitialVoiceControls[voiceIndex];
  }
  for (uint8_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
    midiVoices[voiceIndex].active = false;
    midiVoices[voiceIndex].stackCount = 0;
    arpStates[voiceIndex].enabled = false;
    arpStates[voiceIndex].divisionIndex = 2U;
    arpStates[voiceIndex].stepIndex = 0U;
    arpStates[voiceIndex].clockCounter = 0U;
  }
  midiFramesEnqueued = 0;
  midiFramesAcked = 0;
  midiFramesReplaced = 0;
  midiIgnoredNoteOffs = 0;
  midiResumedNotes = 0;
  midiPreAckLatched = 0;
  midiControlReceived = 0;
  midiControlApplied = 0;
  panelControlApplied = 0;
  controlSchedulePendingWrites = 0;
  controlScheduleInjectedPairs = 0;
  dmcDroppedTriggers = 0;
  dmcAuxDeferrals = 0;
  noteCohortFlushes = 0;
  midiClockTicks = 0;
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
  midiClockRunning = false;
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
      writePattern(STATUS_IDLE_PATTERN);
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
      nextPattern = STATUS_IDLE_PATTERN;
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
      nextPattern = STATUS_IDLE_PATTERN;
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
  setupPanelInputs();
  resetLiveSource();
  resetBusSniffStats();
  writePattern(STATUS_IDLE_PATTERN);

  Serial.begin(115200);
  Serial1.setRX(PIN_MIDI_RX);
  Serial1.begin(MIDI_BAUD);
  delay(250);
  Serial.println();
  Serial.println(F("PicoNesV2B_Step5ACArpPressureMidi start"));
  Serial.println(F("DIN MIDI on GP1, V2B map: CH11=DMC CH12=P1 CH13=P2 CH14=TRI CH15=NOISE CH16=GLOBAL | Step5AC ARP PRESSURE"));
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
    servicePanelInputs(nowMs);
    serviceNoteCohort(micros());
    serviceControlScheduler();
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
