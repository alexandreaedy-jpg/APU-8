#include <Arduino.h>
#include <avr/interrupt.h>
#include <string.h>

/*
  NanoNesMidiMinimal
  ------------------
  Base hardware legere pour APU-8.

  Objectif :
  - garder le transport NES identique au sketch NanoNesPortTest stable
  - ajouter uniquement le MIDI DIN hardware sur RX/D0
  - controles hardware compacts, sans revenir au gros paquet historique

  Cablage NES :
  - Jaune OUT -> D12
  - Noir D0   -> D2
  - D3 NES    -> D3
  - Blanc GND -> GND
  - Vert CLK  -> debranche
  - Rouge 5V  -> non connecte

  MIDI DIN :
  - optocoupleur out -> RX/D0
*/

static const uint8_t NES_CLK_PIN = 2;  // Inutilise ici: le fil CLK vert reste debranche.
static const uint8_t NES_LATCH_PIN = 12;
static const uint8_t NES_DATA_PIN = 2;
static const uint8_t PACKET_SIZE = 12;
static const unsigned long MIDI_BAUD = 31250;
static const unsigned long START_HIGH_US = 1000;
static const uint16_t START_HIGH_TICKS = START_HIGH_US * 2;  // Timer1: 0.5 us/tick.
static const unsigned long MIN_NOTE_HOLD_MS = 0;

static const uint8_t CH_P1 = 12;
static const uint8_t CH_P2 = 13;
static const uint8_t CH_TRI = 14;
static const uint8_t CH_NOISE = 15;
static const uint8_t CH_GLOBAL = 16;
static const bool MIDI_DIAG_RAW_BYTE_TO_P2 = false;
static const bool MIDI_DIAG_ANY_CHANNEL_TO_P2 = false;

// Current live selector mapping:
// D5=P1, D6=P2, D7=TRI, D8=GLOBAL.
static const uint8_t SELECTOR_PINS[4] = {5, 6, 7, 8};
// LFO selector mapping: D9=Pitch, D10=Duty, D11=Amp.
static const uint8_t LFO_TARGET_PINS[3] = {9, 10, 11};
// Active analog controls: A0..A4 = ADSR + duty, A5 = LFO rate, A6 = LFO depth.
static const uint8_t CONTROL_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
static const unsigned long CONTROL_SCAN_MS = 3;
static const unsigned long D3_CONTROL_MIN_SEND_MS = 18;
static const bool FORCE_D3_DIAG_PRESET = false;
static const bool FORCE_D4_DIAG_PRESET = false;
static const bool FORCE_D4_STATIC_HIGH_TEST = false;
static const bool FORCE_CONTROL_TARGET_P2_DIAG = false;
static const uint8_t NES_EVENT_REPEATS = 1;
static const uint8_t CONTROL_COMMAND_REPEATS = 1;
static const uint8_t NES_STARTUP_REPEATS = 8;
static const uint8_t NO_CONTROL_LOCK = 255;
static const unsigned long CONTROL_LOCK_MS = 45;
static const unsigned long CONTROL_NOTE_GUARD_MS = 18;
static const unsigned long ENV_COMMAND_MIN_MS = 28;
static const bool GLOBAL_USES_TRIANGLE = false;
static const uint8_t CTRL_ATTACK = 0;
static const uint8_t CTRL_DECAY = 1;
static const uint8_t CTRL_SUSTAIN = 2;
static const uint8_t CTRL_RELEASE = 3;
// Mapping stable pour ce test:
// A0=Attack, A1=Decay, A2=Sustain, A3=Release
// A4=Duty, A5=Rate, A6=Depth
// A7 libre / non utilisé ici
static const uint8_t CTRL_DUTY = 4;           // A4
static const uint8_t CTRL_LFO_RATE = 5;       // A5
static const uint8_t CTRL_LFO_DEPTH = 6;      // A6
static const uint8_t CTRL_GLIDE = 7;          // A7 libre / réservé
static const uint8_t TARGET_INVALID = 254;
static const uint8_t TARGET_GLOBAL = 255;
static const uint8_t CONTROL_COMMAND_TRIGGER = 0xFE;
static const uint8_t CONTROL_COMMAND_MAGIC = 0x7D;
static const uint8_t CONTROL_PACKET_MAGIC = 0x5A;
static const uint8_t SAFE_PULSE_SUSTAIN_LEVEL = 10;
static const uint8_t PARAM_ATTACK = 0;
static const uint8_t PARAM_DECAY = 1;
static const uint8_t PARAM_SUSTAIN = 2;
static const uint8_t PARAM_RELEASE = 3;
static const uint8_t CONTROL_SCAN_SEQUENCE[] = {
  CTRL_DECAY,
  CTRL_RELEASE,
  CTRL_DECAY,
  CTRL_RELEASE,
  CTRL_ATTACK,
  CTRL_DECAY,
  CTRL_RELEASE,
  CTRL_DUTY,
  CTRL_LFO_DEPTH,
  CTRL_LFO_RATE,
  CTRL_DECAY,
  CTRL_RELEASE
};
static const uint8_t CONTROL_SCAN_SEQUENCE_COUNT = sizeof(CONTROL_SCAN_SEQUENCE) / sizeof(CONTROL_SCAN_SEQUENCE[0]);
// LFO V2 target constants - DOIVENT matcher la ROM V2
// V17 Phase 1 n'utilise réellement que OFF et AMP.
static const uint8_t LFO_TO_OFF = 0;     // Non utilisé dans sketch mais présent pour clarté
static const uint8_t LFO_TO_AMP = 1;    // Phase 1: seul AMP supporté
static const uint8_t LFO_TO_PITCH = 2;  // Phase 2: PITCH (futur)
static const uint8_t LFO_TO_DUTY = 3;   // Phase 2: DUTY (futur)

static const uint8_t VOICE_P1 = 0;
static const uint8_t VOICE_P2 = 1;
static const uint8_t VOICE_TRI = 2;
static const uint8_t VOICE_NOISE = 3;
static const uint8_t VOICE_COUNT = 4;
static const uint8_t GLOBAL_STACK_MAX = 8;
static const uint8_t ARP_VOICE_COUNT = 2;
static const unsigned long ARP_FIXED_INTERVAL_MS = 180;
static const unsigned long ARP_TEMPO_SCAN_MS = 20;
static const unsigned long ARP_MIN_INTERVAL_MS = 45;
static const unsigned long ARP_MAX_INTERVAL_MS = 520;
static const bool ARP_SYNC_FIXED_DIVISION_TEST = false;
static const uint8_t ARP_SYNC_FIXED_CLOCKS_PER_STEP = 6;  // 6 MIDI clocks = double-croche.
static const unsigned long MIDI_CLOCK_DEFAULT_US = 20833UL;  // 120 BPM, 24 PPQN.
static const unsigned long MIDI_CLOCK_MIN_US = 6000UL;
static const unsigned long MIDI_CLOCK_MAX_US = 80000UL;
static const unsigned long MIDI_CLOCK_MAX_ADJUST_US = 1500UL;
static const unsigned long MIDI_CLOCK_ACTIVE_TIMEOUT_MS = 300;
static const uint8_t MIDI_CLOCK_IGNORE_AFTER_TRANSPORT = 2;
static const uint8_t P2_NOTE_QUEUE_SIZE = 16;
static const uint8_t P2_NOTE_QUEUE_MASK = P2_NOTE_QUEUE_SIZE - 1;
static const uint8_t P2_LIVE_MAX_BACKLOG = 2;

struct VoiceState {
  uint8_t note;
  uint8_t vel;
  uint8_t gate;
  uint8_t trigger;
};

struct NoteEntry {
  uint8_t note;
  uint8_t vel;
};

volatile uint8_t activePacket[PACKET_SIZE];
volatile uint8_t latchedPacket[PACKET_SIZE];
volatile uint8_t activePacketD3[PACKET_SIZE];
volatile uint8_t latchedPacketD3[PACKET_SIZE];
volatile uint8_t activePacketD4[PACKET_SIZE];
volatile uint8_t latchedPacketD4[PACKET_SIZE];
volatile uint8_t shiftByteIndex = PACKET_SIZE;
volatile uint8_t shiftBitMask = 1;
volatile bool nesLatchHigh = true;
volatile bool sawOutRise = false;
volatile bool transferActive = false;
volatile uint8_t packetGeneration = 0;
volatile uint8_t sentGeneration = 0;
volatile uint8_t transferGeneration = 0;
volatile uint8_t repeatGeneration = 0;
volatile uint8_t repeatsRemaining = 0;
volatile bool commandActive = false;
volatile uint8_t commandId = 0;
volatile uint8_t commandValue = 0;
volatile uint8_t commandRepeatsRemaining = 0;
volatile uint8_t commandGeneration = 0;
volatile bool transferCommandPacket = false;
volatile uint8_t transferCommandGeneration = 0;
volatile uint8_t voicePacketRepeatsRemaining = 0;
volatile bool transferVoicePacket = false;
volatile uint16_t outRiseTicks = 0;

VoiceState voices[VOICE_COUNT] = {
  {60, 110, 0, 0},
  {64, 110, 0, 0},
  {48, 110, 0, 0},
  {36, 110, 0, 0}
};

NoteEntry globalStack[GLOBAL_STACK_MAX];
uint8_t globalStackCount = 0;
NoteEntry arpHeld[ARP_VOICE_COUNT][GLOBAL_STACK_MAX];
uint8_t arpHeldCount[ARP_VOICE_COUNT] = {0, 0};
uint8_t arpStepIndex[ARP_VOICE_COUNT] = {0, 0};
unsigned long arpNextStepMs[ARP_VOICE_COUNT] = {0, 0};
unsigned long lastArpTempoScanMs = 0;
unsigned long arpIntervalMs = 180;
uint8_t midiClocksPerArpStep = 6;
uint8_t midiClockDivisionIndex = 2;
uint8_t midiClockCounter = 0;
unsigned long lastMidiClockUs = 0;
unsigned long lastMidiClockMs = 0;
unsigned long smoothedMidiClockUs = MIDI_CLOCK_DEFAULT_US;
unsigned long nextSyncedArpStepUs = 0;
uint8_t midiClocksToIgnore = 0;
bool syncedArpStepPending = false;
bool syncedArpRunning = false;
uint8_t busSeq = 1;
uint8_t runningStatus = 0;
uint8_t dataBytes[2];
uint8_t dataIndex = 0;
uint8_t midiDiagStep = 0;
uint8_t currentControlTarget = VOICE_P2;
uint8_t pendingControlTarget = VOICE_P2;
uint8_t pendingControlTargetCount = 0;
uint8_t nextControlIndex = 0;
unsigned long lastControlScanMs = 0;
unsigned long lastNoteEventMs = 0;
unsigned long lastEnvCommandMs = 0;
uint8_t activeControlLock = NO_CONTROL_LOCK;
unsigned long controlLockUntilMs = 0;
int filteredControlRaw[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t lastControlValue[8] = {255, 255, 255, 255, 255, 255, 255, 255};
uint8_t controlPickupReference[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool controlPickupArmed[8] = {false, false, false, false, false, false, false, false};
uint8_t pulseDuty[2] = {2, 2};
uint8_t pulseLevel[2] = {15, 15};
uint8_t envAttack[2] = {0, 0};
uint8_t envDecay[2] = {0, 0};
uint8_t envSustain[2] = {127, 127};
uint8_t envRelease[2] = {0, 0};
// LFO V2 defaults: AMP target (Phase 1), rate=0 (off), depth=0
uint8_t voiceLfoDepth[3] = {0, 0, 0};
uint8_t voiceLfoRate[3] = {0, 0, 0};
uint8_t voiceLfoTarget[3] = {LFO_TO_AMP, LFO_TO_AMP, LFO_TO_OFF};
uint8_t currentLfoTarget = LFO_TO_AMP;  // Phase 1: AMP seul
uint8_t pendingLfoTarget = LFO_TO_AMP;
uint8_t pendingLfoTargetCount = 0;
unsigned long voiceHoldUntilMs[VOICE_COUNT] = {0, 0, 0, 0};
bool voiceOffPending[VOICE_COUNT] = {false, false, false, false};
unsigned long lastD3ControlSendMs = 0;

volatile uint8_t p2OutNote = 64;
volatile uint8_t p2OutVel = 110;
volatile uint8_t p2OutGate = 0;
volatile uint8_t p2OutTrigger = 0;
volatile uint8_t p2QueuedNotes[P2_NOTE_QUEUE_SIZE];
volatile uint8_t p2QueuedVels[P2_NOTE_QUEUE_SIZE];
volatile uint8_t p2QueueHead = 0;
volatile uint8_t p2QueueTail = 0;
volatile uint8_t p2QueueCount = 0;
volatile bool p2GateOffAfterQueue = false;

uint8_t p2HeldNotes[GLOBAL_STACK_MAX];
uint8_t p2HeldCount = 0;

void writeP2PacketFromState();
void writeVoicePacketFromState();
void writeD3PacketFromState();
void writeD4PacketFromState();
void writeCommandPacketFromState();
void prepareNextP2Packet();
void setP2Live(uint8_t note, uint8_t vel, bool gate);
inline void setNesDataBit(bool high);
inline void setNesD3Bit(bool high);
inline void setNesD4Bit(bool high);
inline void markCurrentPacketDirty(uint8_t repeats);
inline void cancelEnvelopeCommandState();
void cancelEnvelopeCommand();
bool anyVoiceGateActive();
void queueEnvelopeCommand(uint8_t target, uint8_t param, uint8_t value);
void setupControls();
void processControls();
void serviceArpeggiator();
bool syncEnabled();
bool syncedClockActive();
void handleMidiRealtime(uint8_t b);
void clockArpeggiatorStep();
void serviceArpClockDivisionControl();
void serviceSyncedArpeggiator();
void resetSyncedArpClock();
void resetSyncedArpPatternPhase();
void softenSyncedArpClock(bool resetPhase);
void observeMidiClockTick();
unsigned long syncedArpStepIntervalUs();
void setVoiceState(uint8_t voiceId, uint8_t note, uint8_t vel, bool gate, unsigned long nowMs);
bool triggerArpStepState(uint8_t voiceId, uint8_t arpIndex, unsigned long nowMs);
uint8_t velocityTierFromMidi(uint8_t vel);
uint8_t velocityNibbleFromMidi(uint8_t vel);
uint8_t ctrlNibbleFromMidi(uint8_t value);
bool applyEnvelopeControl(uint8_t target, uint8_t param, uint8_t value, uint8_t* values);
bool applyLfoControl(uint8_t target, uint8_t* values, uint8_t value);
uint8_t lfoNibbleFromMidiStable(uint8_t value, uint8_t current, uint8_t deadzone);
void forceControlRescan();
void armControlPickupForTargetChange();
uint8_t readLfoTargetSelector();
bool applyLfoTarget(uint8_t target, uint8_t mode);

inline void outputIdleEventBit() {
  setNesDataBit(packetGeneration != sentGeneration ? 1 : 0);
  setNesD3Bit(0);
  setNesD4Bit(0);
}

inline void setNesDataBit(bool high) {
  DDRD |= _BV(PD2);  // D2 → DN0 (Notes MIDI traitées)
  if (high) {
    PORTD |= _BV(PD2);
  } else {
    PORTD &= (uint8_t)~_BV(PD2);
  }
}

inline void setNesD3Bit(bool high) {
  DDRD |= _BV(PD3);  // D3 → DN3 (Contrôles)
  if (high) {
    PORTD |= _BV(PD3);
  } else {
    PORTD &= (uint8_t)~_BV(PD3);
  }
}

inline void setNesD4Bit(bool high) {
  DDRD |= _BV(PD4);  // D4 -> DN4 (LFO)
  if (high) {
    PORTD |= _BV(PD4);
  } else {
    PORTD &= (uint8_t)~_BV(PD4);
  }
}

inline void outputCurrentLatchedBit() {
  if (shiftByteIndex >= PACKET_SIZE) {
    outputIdleEventBit();
    return;
  }

  setNesDataBit((latchedPacket[shiftByteIndex] & shiftBitMask) != 0);
  setNesD3Bit((latchedPacketD3[shiftByteIndex] & shiftBitMask) != 0);
  setNesD4Bit((latchedPacketD4[shiftByteIndex] & shiftBitMask) != 0);
}

inline void beginControllerTransfer() {
  transferActive = true;
  transferVoicePacket = voicePacketRepeatsRemaining > 0;
  // Keep D0 on the stable "voice packet" format and layer controls on D3.
  writeVoicePacketFromState();
  writeD3PacketFromState();
  writeD4PacketFromState();
  transferGeneration = packetGeneration;
  transferCommandPacket = false;
  transferCommandGeneration = commandGeneration;
  for (uint8_t i = 0; i < PACKET_SIZE; ++i) {
    latchedPacket[i] = activePacket[i];
    latchedPacketD3[i] = activePacketD3[i];
    latchedPacketD4[i] = activePacketD4[i];
  }
  shiftByteIndex = 0;
  shiftBitMask = 1;
  outputCurrentLatchedBit();
}

inline void advanceControllerBit() {
  shiftBitMask <<= 1;
  if (shiftBitMask == 0) {
    shiftBitMask = 1;
    ++shiftByteIndex;
    if (shiftByteIndex >= PACKET_SIZE) {
      shiftByteIndex = PACKET_SIZE;
      transferActive = false;
      if (transferCommandPacket && transferCommandGeneration == commandGeneration && commandRepeatsRemaining > 0) {
        --commandRepeatsRemaining;
        if (commandRepeatsRemaining == 0) {
          commandActive = false;
        }
      }
      transferCommandPacket = false;
      if (transferVoicePacket && voicePacketRepeatsRemaining > 0) {
        --voicePacketRepeatsRemaining;
      }
      transferVoicePacket = false;
      if (transferGeneration == repeatGeneration && repeatsRemaining > 0) {
        --repeatsRemaining;
        if (repeatsRemaining == 0) {
          sentGeneration = transferGeneration;
        }
      } else if (transferGeneration == packetGeneration) {
        sentGeneration = transferGeneration;
      }
      outputIdleEventBit();
      return;
    }
  }

  outputCurrentLatchedBit();
}

ISR(PCINT0_vect) {
  const bool latchHigh = (PINB & _BV(PB4)) != 0;
  const uint16_t now = TCNT1;

  if (latchHigh && !nesLatchHigh) {
    outRiseTicks = now;
    sawOutRise = true;
  } else if (!latchHigh && nesLatchHigh) {
    if (sawOutRise) {
      sawOutRise = false;
      if (!transferActive) {
        if (packetGeneration != sentGeneration) {
          beginControllerTransfer();
        }
      } else {
        advanceControllerBit();
      }
    }
  }

  nesLatchHigh = latchHigh;
}

void rebuildPayload() {
  noInterrupts();
  writeP2PacketFromState();
  writeD3PacketFromState();  // Reconstruire D3 immédiatement aussi
  writeD4PacketFromState();
  markCurrentPacketDirty(NES_EVENT_REPEATS);
  interrupts();
}

void cancelEnvelopeCommand() {
  noInterrupts();
  cancelEnvelopeCommandState();
  interrupts();
}

inline void cancelEnvelopeCommandState() {
  commandActive = false;
  commandRepeatsRemaining = 0;
  transferCommandPacket = false;
}

bool anyVoiceGateActive() {
  return voices[VOICE_P1].gate ||
         voices[VOICE_P2].gate ||
         voices[VOICE_TRI].gate ||
         voices[VOICE_NOISE].gate;
}

inline void markCurrentPacketDirty(uint8_t repeats) {
  ++packetGeneration;
  if (packetGeneration == sentGeneration) {
    ++packetGeneration;
  }
  repeatGeneration = packetGeneration;
  repeatsRemaining = repeats ? repeats : 1;
  if (!transferActive) {
    outputIdleEventBit();
  }
}

void writeP2PacketFromState() {
  uint8_t p1VelocityTier = velocityTierFromMidi(voices[VOICE_P1].vel);
  uint8_t p2VelocityTier = velocityTierFromMidi(voices[VOICE_P2].vel);

  activePacket[0] = 0xA8;
  activePacket[1] = CONTROL_PACKET_MAGIC;
  activePacket[2] = voices[VOICE_P1].trigger;
  activePacket[3] = (voices[VOICE_P1].gate ? 0x80 : 0x00) | (voices[VOICE_P1].note & 0x7F);
  activePacket[4] = voices[VOICE_P2].trigger;
  activePacket[5] = (voices[VOICE_P2].gate ? 0x80 : 0x00) | (voices[VOICE_P2].note & 0x7F);
  // TRI ajouté ici pour le format 3 voix (P1+P2+TRI)
  activePacket[6] = voices[VOICE_TRI].trigger;
  activePacket[7] = (voices[VOICE_TRI].gate ? 0x80 : 0x00) | (voices[VOICE_TRI].note & 0x7F);
  // NOISE désactivé pour l'instant (prévu pour plus tard)
  activePacket[8] = 0;
  // LFO V2: LFO data removed from D0 - now exclusively in D4 packet
  // Bytes 9-11: reserved for future use, currently 0
  activePacket[9] = 0;
  activePacket[10] = (uint8_t)(((p1VelocityTier & 0x03) << 2) | (pulseDuty[0] & 0x03));
  activePacket[11] = (uint8_t)(((p2VelocityTier & 0x03) << 2) | (pulseDuty[1] & 0x03));
}

void writeD3PacketFromState() {
  activePacketD3[0] = 0xD3;
  activePacketD3[1] = 0x3D;
  activePacketD3[2] = (uint8_t)((ctrlNibbleFromMidi(envAttack[VOICE_P1]) << 4) | ctrlNibbleFromMidi(envAttack[VOICE_P2]));     // A0
  activePacketD3[3] = (uint8_t)((ctrlNibbleFromMidi(envDecay[VOICE_P1]) << 4) | ctrlNibbleFromMidi(envDecay[VOICE_P2]));       // A1
  activePacketD3[4] = (uint8_t)((ctrlNibbleFromMidi(envSustain[VOICE_P1]) << 4) | ctrlNibbleFromMidi(envSustain[VOICE_P2]));      // A2
  activePacketD3[5] = (uint8_t)((ctrlNibbleFromMidi(envRelease[VOICE_P1]) << 4) | ctrlNibbleFromMidi(envRelease[VOICE_P2]));     // A3
  activePacketD3[6] = (uint8_t)((pulseDuty[0] << 4) | (pulseDuty[1] & 0x0F));  // A4 Duty P1/P2
  for (uint8_t i = 7; i < PACKET_SIZE; ++i) {
    activePacketD3[i] = 0;
  }
}

// LFO V2: New D4 packet format
// Header: 0xD4 0xB4
// Byte 2: seq (increments when config changes)
// P1 config: byte 3 (mode[7:6] | target[5:4] | depth[3:0]), byte 4 (rate[7:4] | 0[3:0])
// P2 config: byte 5, byte 6
// TRI config: byte 7, byte 8
// Global flags: byte 9 (bit 4 = require double-packet latch)
// Reserved: byte 10
// Checksum: byte 11 (XOR of bytes 0-10)

static uint8_t d4v2_seq = 0;
static uint8_t d4v2_prev_config[6] = {0};  // P1 cfg A/B, P2 cfg A/B, TRI cfg A/B

void writeD4PacketFromState() {
  // Build new config
  uint8_t new_config[6];
  // P1: mode=0 (free-run), target from voiceLfoTarget, depth 0-15, rate 0-15
  uint8_t p1_mode = 0;  // free-run default
  uint8_t p1_target = (voiceLfoTarget[VOICE_P1] == LFO_TO_AMP) ? LFO_TO_AMP : LFO_TO_OFF;
  uint8_t p1_depth = voiceLfoDepth[VOICE_P1] & 0x0F;
  uint8_t p1_rate = voiceLfoRate[VOICE_P1] & 0x0F;
  new_config[0] = (uint8_t)((p1_mode << 6) | (p1_target << 4) | p1_depth);  // Config A
  new_config[1] = (uint8_t)(p1_rate << 4);  // Config B

  // P2
  uint8_t p2_mode = 0;
  uint8_t p2_target = (voiceLfoTarget[VOICE_P2] == LFO_TO_AMP) ? LFO_TO_AMP : LFO_TO_OFF;
  uint8_t p2_depth = voiceLfoDepth[VOICE_P2] & 0x0F;
  uint8_t p2_rate = voiceLfoRate[VOICE_P2] & 0x0F;
  new_config[2] = (uint8_t)((p2_mode << 6) | (p2_target << 4) | p2_depth);
  new_config[3] = (uint8_t)(p2_rate << 4);

  // TRI (Phase 1: TRI not supported, always off)
  new_config[4] = 0;  // Config A: mode=0, target=0 (off), depth=0
  new_config[5] = 0;  // Config B: rate=0

  // Check if config changed
  bool config_changed = false;
  for (uint8_t i = 0; i < 6; i++) {
    if (new_config[i] != d4v2_prev_config[i]) {
      config_changed = true;
      d4v2_prev_config[i] = new_config[i];
    }
  }

  if (config_changed) {
    d4v2_seq++;
  }

  // Build packet
  activePacketD4[0] = 0xD4;  // Header 0
  activePacketD4[1] = 0xB4;  // Header 1
  activePacketD4[2] = d4v2_seq;  // Sequence number
  activePacketD4[3] = new_config[0];  // P1 config A
  activePacketD4[4] = new_config[1];  // P1 config B
  activePacketD4[5] = new_config[2];  // P2 config A
  activePacketD4[6] = new_config[3];  // P2 config B
  activePacketD4[7] = new_config[4];  // TRI config A
  activePacketD4[8] = new_config[5];  // TRI config B
  activePacketD4[9] = 0x10;  // Global flags: bit 4 set (require double-packet latch)
  activePacketD4[10] = 0;    // Reserved

  // Compute checksum (XOR of bytes 0-10)
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < 11; i++) {
    checksum ^= activePacketD4[i];
  }
  activePacketD4[11] = checksum;

  // Fill rest with zeros (if PACKET_SIZE > 12)
  for (uint8_t i = 12; i < PACKET_SIZE; ++i) {
    activePacketD4[i] = 0;
  }
}

void writeVoicePacketFromState() {
  activePacket[0] = 0xA8;
  activePacket[1] = 0x58;
  activePacket[2] = voices[VOICE_P1].trigger;
  activePacket[3] = (voices[VOICE_P1].gate ? 0x80 : 0x00) | (voices[VOICE_P1].note & 0x7F);
  activePacket[4] = voices[VOICE_P2].trigger;
  activePacket[5] = (voices[VOICE_P2].gate ? 0x80 : 0x00) | (voices[VOICE_P2].note & 0x7F);
  activePacket[6] = voices[VOICE_TRI].trigger;
  activePacket[7] = (voices[VOICE_TRI].gate ? 0x80 : 0x00) | (voices[VOICE_TRI].note & 0x7F);
  activePacket[8] = voices[VOICE_NOISE].trigger;
  activePacket[9] = (voices[VOICE_NOISE].gate ? 0x80 : 0x00) | (voices[VOICE_NOISE].note & 0x7F);
  activePacket[10] = (uint8_t)((velocityNibbleFromMidi(voices[VOICE_P1].vel) << 4) | (pulseDuty[0] & 0x03));
  activePacket[11] = (uint8_t)((velocityNibbleFromMidi(voices[VOICE_P2].vel) << 4) | (pulseDuty[1] & 0x03));
}

void writeCommandPacketFromState() {
  activePacket[0] = 0xA8;
  activePacket[1] = 0x58;
  activePacket[2] = voices[VOICE_P1].trigger;
  activePacket[3] = (voices[VOICE_P1].gate ? 0x80 : 0x00) | (voices[VOICE_P1].note & 0x7F);
  activePacket[4] = voices[VOICE_P2].trigger;
  activePacket[5] = (voices[VOICE_P2].gate ? 0x80 : 0x00) | (voices[VOICE_P2].note & 0x7F);
  activePacket[6] = 0;
  activePacket[7] = 0;
  activePacket[8] = CONTROL_COMMAND_TRIGGER;
  activePacket[9] = CONTROL_COMMAND_MAGIC;
  activePacket[10] = commandId;
  activePacket[11] = commandValue;
}

uint8_t velocityTierFromMidi(uint8_t vel) {
  if (vel < 32) return 0;
  if (vel < 64) return 1;
  if (vel < 96) return 2;
  return 3;
}

uint8_t velocityNibbleFromMidi(uint8_t vel) {
  return (uint8_t)(((unsigned int)vel * 15U + 63U) / 127U);
}

uint8_t ctrlNibbleFromMidi(uint8_t value) {
  return (uint8_t)(((unsigned int)value * 15U + 63U) / 127U);
}

void prepareNextP2Packet() {
  writeP2PacketFromState();
  writeD3PacketFromState();
  writeD4PacketFromState();
}

void setP2Live(uint8_t note, uint8_t vel, bool gate) {
  lastNoteEventMs = millis();
  noInterrupts();
  p2OutNote = note & 0x7F;
  p2OutVel = vel == 0 ? 1 : vel;
  p2OutGate = gate ? 1 : 0;
  if (gate) {
    ++p2OutTrigger;
  }
  writeP2PacketFromState();
  interrupts();
}

void enqueueP2NoteOn(uint8_t note, uint8_t vel) {
  noInterrupts();
  while (p2QueueCount >= P2_LIVE_MAX_BACKLOG) {
    p2QueueTail = (p2QueueTail + 1) & P2_NOTE_QUEUE_MASK;
    --p2QueueCount;
  }
  if (p2QueueCount >= P2_NOTE_QUEUE_SIZE) {
    p2QueueTail = (p2QueueTail + 1) & P2_NOTE_QUEUE_MASK;
    --p2QueueCount;
  }
  p2QueuedNotes[p2QueueHead] = note;
  p2QueuedVels[p2QueueHead] = vel == 0 ? 1 : vel;
  p2QueueHead = (p2QueueHead + 1) & P2_NOTE_QUEUE_MASK;
  ++p2QueueCount;
  p2GateOffAfterQueue = false;
  interrupts();
}

void requestP2GateOffAfterQueue() {
  noInterrupts();
  p2GateOffAfterQueue = true;
  interrupts();
}

void addP2HeldNote(uint8_t note) {
  for (uint8_t i = 0; i < p2HeldCount; ++i) {
    if (p2HeldNotes[i] == note) {
      return;
    }
  }
  if (p2HeldCount < GLOBAL_STACK_MAX) {
    p2HeldNotes[p2HeldCount++] = note;
  }
}

void removeP2HeldNote(uint8_t note) {
  for (uint8_t i = 0; i < p2HeldCount; ++i) {
    if (p2HeldNotes[i] == note) {
      for (uint8_t j = i; (uint8_t)(j + 1) < p2HeldCount; ++j) {
        p2HeldNotes[j] = p2HeldNotes[j + 1];
      }
      --p2HeldCount;
      return;
    }
  }
}

uint8_t voiceFromChannel(uint8_t channel) {
  if (channel == CH_P1) return VOICE_P1;
  if (channel == CH_P2) return VOICE_P2;
  if (channel == CH_TRI) return VOICE_TRI;
  return VOICE_NOISE;
}

void setVoice(uint8_t voiceId, uint8_t note, uint8_t vel, bool gate) {
  unsigned long nowMs = millis();
  setVoiceState(voiceId, note, vel, gate, nowMs);
  if (voiceId == VOICE_TRI) {
    noInterrupts();
    voicePacketRepeatsRemaining = NES_EVENT_REPEATS;
    interrupts();
  }
  rebuildPayload();
}

void setVoiceState(uint8_t voiceId, uint8_t note, uint8_t vel, bool gate, unsigned long nowMs) {
  lastNoteEventMs = nowMs;
  voices[voiceId].note = note;
  voices[voiceId].vel = vel == 0 ? 1 : vel;
  voices[voiceId].gate = gate ? 1 : 0;
  if (gate) {
    ++voices[voiceId].trigger;
    voiceHoldUntilMs[voiceId] = nowMs + MIN_NOTE_HOLD_MS;
    voiceOffPending[voiceId] = false;
  }
}

uint8_t arpIndexFromVoice(uint8_t voiceId) {
  if (voiceId == VOICE_P1) return 0;
  if (voiceId == VOICE_P2) return 1;
  return 255;
}

unsigned long readArpIntervalMs(unsigned long nowMs) {
  (void)nowMs;
  return ARP_FIXED_INTERVAL_MS;
}

void serviceArpClockDivisionControl() {
  return;
}

void triggerArpStep(uint8_t voiceId, uint8_t arpIndex) {
  if (triggerArpStepState(voiceId, arpIndex, millis())) {
    rebuildPayload();
  }
}

bool triggerArpStepState(uint8_t voiceId, uint8_t arpIndex, unsigned long nowMs) {
  if (arpHeldCount[arpIndex] == 0) {
    return false;
  }
  if (arpStepIndex[arpIndex] >= arpHeldCount[arpIndex]) {
    arpStepIndex[arpIndex] = 0;
  }

  NoteEntry step = arpHeld[arpIndex][arpStepIndex[arpIndex]];
  arpStepIndex[arpIndex] = (arpStepIndex[arpIndex] + 1) % arpHeldCount[arpIndex];
  setVoiceState(voiceId, step.note, step.vel, true, nowMs);
  return true;
}

void addArpNote(uint8_t voiceId, uint8_t note, uint8_t vel) {
  uint8_t arpIndex = arpIndexFromVoice(voiceId);
  if (arpIndex == 255) {
    setVoice(voiceId, note, vel, true);
    return;
  }

  bool syncNow = syncedClockActive();
  uint8_t previousHeldCount = arpHeldCount[arpIndex];

  for (uint8_t i = 0; i < arpHeldCount[arpIndex]; ++i) {
    if (arpHeld[arpIndex][i].note == note) {
      arpHeld[arpIndex][i].vel = vel;
      if (!syncNow) {
        setVoice(voiceId, note, vel, true);
      }
      return;
    }
  }

  if (arpHeldCount[arpIndex] < GLOBAL_STACK_MAX) {
    arpHeld[arpIndex][arpHeldCount[arpIndex]].note = note;
    arpHeld[arpIndex][arpHeldCount[arpIndex]].vel = vel;
    ++arpHeldCount[arpIndex];
  }

  if (syncNow) {
    if (previousHeldCount == 0 || arpStepIndex[arpIndex] >= arpHeldCount[arpIndex]) {
      arpStepIndex[arpIndex] = 0;
    }
    arpNextStepMs[arpIndex] = millis();
    return;
  }

  arpStepIndex[arpIndex] = arpHeldCount[arpIndex] > 1 ? (arpHeldCount[arpIndex] - 1) : 0;
  arpNextStepMs[arpIndex] = millis();
  triggerArpStep(voiceId, arpIndex);
}

void removeArpNote(uint8_t voiceId, uint8_t note) {
  uint8_t arpIndex = arpIndexFromVoice(voiceId);
  if (arpIndex == 255) {
    releaseVoice(voiceId, note);
    return;
  }

  for (uint8_t i = 0; i < arpHeldCount[arpIndex]; ++i) {
    if (arpHeld[arpIndex][i].note == note) {
      for (uint8_t j = i; (uint8_t)(j + 1) < arpHeldCount[arpIndex]; ++j) {
        arpHeld[arpIndex][j] = arpHeld[arpIndex][j + 1];
      }
      --arpHeldCount[arpIndex];
      if (arpStepIndex[arpIndex] > i && arpStepIndex[arpIndex] > 0) {
        --arpStepIndex[arpIndex];
      }
      break;
    }
  }

  if (arpHeldCount[arpIndex] == 0) {
    releaseVoice(voiceId, voices[voiceId].note);
    return;
  }

  if (arpStepIndex[arpIndex] >= arpHeldCount[arpIndex]) {
    arpStepIndex[arpIndex] = 0;
  }
  if (syncedClockActive()) {
    if (voices[voiceId].gate && voices[voiceId].note == note) {
      releaseVoice(voiceId, note);
    }
    return;
  }
  arpNextStepMs[arpIndex] = millis();
  triggerArpStep(voiceId, arpIndex);
}

void releaseVoice(uint8_t voiceId, uint8_t note) {
  if (voices[voiceId].note != note) {
    return;
  }

  if ((long)(millis() - voiceHoldUntilMs[voiceId]) < 0) {
    voiceOffPending[voiceId] = true;
    return;
  }

  voices[voiceId].gate = 0;
  voiceOffPending[voiceId] = false;
  lastNoteEventMs = millis();
  if (voiceId == VOICE_TRI) {
    noInterrupts();
    voicePacketRepeatsRemaining = NES_EVENT_REPEATS;
    interrupts();
  }
  rebuildPayload();
}

void serviceArpeggiator() {
  unsigned long nowMs = millis();
  unsigned long intervalMs = readArpIntervalMs(nowMs);

  for (uint8_t arpIndex = 0; arpIndex < ARP_VOICE_COUNT; ++arpIndex) {
    if (arpHeldCount[arpIndex] < 2) {
      continue;
    }
    if ((long)(nowMs - arpNextStepMs[arpIndex]) < 0) {
      continue;
    }

    uint8_t voiceId = arpIndex == 0 ? VOICE_P1 : VOICE_P2;
    triggerArpStep(voiceId, arpIndex);
    arpNextStepMs[arpIndex] = nowMs + intervalMs;
  }
}

void clockArpeggiatorStep() {
  bool changed = false;
  unsigned long nowMs = millis();

  noInterrupts();
  for (uint8_t arpIndex = 0; arpIndex < ARP_VOICE_COUNT; ++arpIndex) {
    if (arpHeldCount[arpIndex] == 0) {
      continue;
    }
    uint8_t voiceId = arpIndex == 0 ? VOICE_P1 : VOICE_P2;
    if (arpHeldCount[arpIndex] == 1) {
      NoteEntry step = arpHeld[arpIndex][0];
      if (!voices[voiceId].gate || voices[voiceId].note != step.note || voices[voiceId].vel != step.vel) {
        setVoiceState(voiceId, step.note, step.vel, true, nowMs);
        changed = true;
      }
    } else if (triggerArpStepState(voiceId, arpIndex, nowMs)) {
      changed = true;
    }
  }
  if (changed) {
    writeP2PacketFromState();
    markCurrentPacketDirty(NES_EVENT_REPEATS);
  }
  interrupts();
}

void resetSyncedArpClock() {
  midiClockCounter = 0;
  lastMidiClockUs = 0;
  lastMidiClockMs = 0;
  nextSyncedArpStepUs = 0;
  midiClocksToIgnore = 0;
  syncedArpStepPending = false;
  syncedArpRunning = false;
}

void resetSyncedArpPatternPhase() {
  midiClockCounter = 0;
  nextSyncedArpStepUs = 0;
  syncedArpStepPending = false;
  syncedArpRunning = false;
  for (uint8_t arpIndex = 0; arpIndex < ARP_VOICE_COUNT; ++arpIndex) {
    arpStepIndex[arpIndex] = 0;
  }
}

void softenSyncedArpClock(bool resetPhase) {
  midiClocksToIgnore = MIDI_CLOCK_IGNORE_AFTER_TRANSPORT;
  lastMidiClockUs = 0;
  lastMidiClockMs = 0;
  if (resetPhase) {
    resetSyncedArpPatternPhase();
  }
}

unsigned long syncedArpStepIntervalUs() {
  uint8_t division = ARP_SYNC_FIXED_DIVISION_TEST ? ARP_SYNC_FIXED_CLOCKS_PER_STEP : midiClocksPerArpStep;
  if (division == 0) {
    division = 6;
  }
  return smoothedMidiClockUs * (unsigned long)division;
}

void observeMidiClockTick() {
  unsigned long nowUs = micros();
  lastMidiClockMs = millis();
  bool useForTempo = true;
  uint8_t division = ARP_SYNC_FIXED_DIVISION_TEST ? ARP_SYNC_FIXED_CLOCKS_PER_STEP : midiClocksPerArpStep;
  if (division == 0) {
    division = 6;
  }

  if (midiClocksToIgnore > 0) {
    --midiClocksToIgnore;
    useForTempo = false;
  }

  if (useForTempo && lastMidiClockUs != 0) {
    unsigned long deltaUs = nowUs - lastMidiClockUs;
    if (deltaUs >= MIDI_CLOCK_MIN_US && deltaUs <= MIDI_CLOCK_MAX_US) {
      unsigned long lowerUs = smoothedMidiClockUs > MIDI_CLOCK_MAX_ADJUST_US
                                ? smoothedMidiClockUs - MIDI_CLOCK_MAX_ADJUST_US
                                : MIDI_CLOCK_MIN_US;
      unsigned long upperUs = smoothedMidiClockUs + MIDI_CLOCK_MAX_ADJUST_US;
      if (lowerUs < MIDI_CLOCK_MIN_US) {
        lowerUs = MIDI_CLOCK_MIN_US;
      }
      if (upperUs > MIDI_CLOCK_MAX_US) {
        upperUs = MIDI_CLOCK_MAX_US;
      }
      if (deltaUs < lowerUs) {
        deltaUs = lowerUs;
      } else if (deltaUs > upperUs) {
        deltaUs = upperUs;
      }
      smoothedMidiClockUs = ((smoothedMidiClockUs * 7UL) + deltaUs + 4UL) / 8UL;
    }
  }
  lastMidiClockUs = nowUs;

  ++midiClockCounter;
  if (midiClockCounter >= division) {
    unsigned long intervalUs = syncedArpStepIntervalUs();
    midiClockCounter = 0;

    if (!syncedArpRunning) {
      nextSyncedArpStepUs = nowUs;
      syncedArpRunning = true;
    } else {
      long phaseErrorUs = (long)(nowUs - nextSyncedArpStepUs);
      if (phaseErrorUs > 12000L || phaseErrorUs < -12000L) {
        nextSyncedArpStepUs = nowUs;
      } else {
        nextSyncedArpStepUs += phaseErrorUs / 4L;
      }
    }
    syncedArpStepPending = true;
  }
}

void serviceSyncedArpeggiator() {
  return;
}

void serviceHeldNoteOffs() {
  for (uint8_t i = 0; i < VOICE_COUNT; ++i) {
    if (voiceOffPending[i] && (long)(millis() - voiceHoldUntilMs[i]) >= 0) {
      voices[i].gate = 0;
      voiceOffPending[i] = false;
      lastNoteEventMs = millis();
      rebuildPayload();
    }
  }
}

void refreshGlobalVoices() {
  bool changed = false;
  lastNoteEventMs = millis();
  if (globalStackCount > 0) {
    if (!voices[VOICE_P1].gate || voices[VOICE_P1].note != globalStack[0].note) {
      ++voices[VOICE_P1].trigger;
      changed = true;
    }
    voices[VOICE_P1].note = globalStack[0].note;
    voices[VOICE_P1].vel = globalStack[0].vel == 0 ? 1 : globalStack[0].vel;
    voices[VOICE_P1].gate = 1;
  } else {
    if (voices[VOICE_P1].gate) {
      voices[VOICE_P1].gate = 0;
      changed = true;
    }
  }

  if (globalStackCount > 1) {
    if (!voices[VOICE_P2].gate || voices[VOICE_P2].note != globalStack[1].note) {
      ++voices[VOICE_P2].trigger;
      changed = true;
    }
    voices[VOICE_P2].note = globalStack[1].note;
    voices[VOICE_P2].vel = globalStack[1].vel == 0 ? 1 : globalStack[1].vel;
    voices[VOICE_P2].gate = 1;
  } else {
    if (voices[VOICE_P2].gate) {
      voices[VOICE_P2].gate = 0;
      changed = true;
    }
  }

  if (GLOBAL_USES_TRIANGLE && globalStackCount > 2) {
    if (!voices[VOICE_TRI].gate || voices[VOICE_TRI].note != globalStack[2].note) {
      ++voices[VOICE_TRI].trigger;
      changed = true;
    }
    voices[VOICE_TRI].note = globalStack[2].note;
    voices[VOICE_TRI].vel = globalStack[2].vel == 0 ? 1 : globalStack[2].vel;
    voices[VOICE_TRI].gate = 1;
  } else if (voices[VOICE_TRI].gate && GLOBAL_USES_TRIANGLE) {
    voices[VOICE_TRI].gate = 0;
    changed = true;
  }

  voiceOffPending[VOICE_P1] = false;
  voiceOffPending[VOICE_P2] = false;
  voiceOffPending[VOICE_TRI] = false;
  if (changed) {
    rebuildPayload();
  }
}

void addGlobalNote(uint8_t note, uint8_t vel) {
  for (uint8_t i = 0; i < globalStackCount; ++i) {
    if (globalStack[i].note == note) {
      globalStack[i].vel = vel;
      return;
    }
  }
  if (globalStackCount < GLOBAL_STACK_MAX) {
    globalStack[globalStackCount].note = note;
    globalStack[globalStackCount].vel = vel;
    ++globalStackCount;
  }
}

void setupTimingTimer() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = _BV(CS11);  // Prescaler 8: 16 MHz / 8 = 2 MHz.
  TCNT1 = 0;
  interrupts();
}

uint8_t mapRawToMidi(int raw) {
  if (raw < 0) raw = 0;
  if (raw > 1023) raw = 1023;
  return (uint8_t)((raw * 127L) / 1023L);
}

bool syncEnabled() {
  return false;
}

bool syncedClockActive() {
  return false;
}

uint8_t readControlTarget() {
  if (FORCE_CONTROL_TARGET_P2_DIAG) {
    return VOICE_P2;
  }

  uint8_t lowMask = 0;
  uint8_t activeCount = 0;

  if (digitalRead(SELECTOR_PINS[0]) == LOW) lowMask |= 0x01;
  if (digitalRead(SELECTOR_PINS[1]) == LOW) lowMask |= 0x02;
  if (digitalRead(SELECTOR_PINS[2]) == LOW) lowMask |= 0x04;
  if (digitalRead(SELECTOR_PINS[3]) == LOW) lowMask |= 0x08;

  // Hardware workaround: if P1 stays LOW parasitically while another selector
  // position is active, ignore P1 and let the other explicit target win.
  if ((lowMask & 0x01) && (lowMask & 0x0E)) {
    lowMask &= (uint8_t)~0x01;
  }

  if (lowMask & 0x01) ++activeCount;
  if (lowMask & 0x02) ++activeCount;
  if (lowMask & 0x04) ++activeCount;
  if (lowMask & 0x08) ++activeCount;

  if (activeCount == 1) {
    if (lowMask & 0x01) return VOICE_P1;
    if (lowMask & 0x02) return VOICE_P2;
    if (lowMask & 0x04) return VOICE_TRI;
    if (lowMask & 0x08) return TARGET_GLOBAL;
  }

  return TARGET_INVALID;
}

// LFO V2 Phase 1: seul AMP est réellement supporté.
// Les positions PITCH/DUTY restent câblées pour la suite, mais doivent être
// traitées comme OFF dans cette branche pour éviter les faux comportements.
uint8_t readLfoTargetSelector() {
  if (digitalRead(LFO_TARGET_PINS[2]) == LOW) return LFO_TO_AMP;
  if (digitalRead(LFO_TARGET_PINS[0]) == LOW) return LFO_TO_OFF;
  if (digitalRead(LFO_TARGET_PINS[1]) == LOW) return LFO_TO_OFF;
  return currentLfoTarget;
}

int readAnalogLight(uint8_t pin) {
  analogRead(pin);
  delayMicroseconds(80);
  int a = analogRead(pin);
  int b = analogRead(pin);
  int c = analogRead(pin);
  return (a + b + c + 1) / 3;
}

bool setVoiceValue(uint8_t* values, uint8_t voice, uint8_t value) {
  if (values[voice] == value) {
    return false;
  }
  values[voice] = value;
  return true;
}

bool applyPolyControl(uint8_t target, uint8_t* values, uint8_t value, bool attenuateTriangle) {
  bool changed = false;
  uint8_t triValue = attenuateTriangle ? (value >> 2) : value;

  if (target == VOICE_P1 || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(values, VOICE_P1, value);
  }
  if (target == VOICE_P2 || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(values, VOICE_P2, value);
  }
  if (target == VOICE_TRI || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(values, VOICE_TRI, triValue);
  }

  return changed;
}

bool applyLfoTarget(uint8_t target, uint8_t mode) {
  bool changed = false;
  uint8_t normalizedMode = (mode == LFO_TO_AMP) ? LFO_TO_AMP : LFO_TO_OFF;

  if (target == VOICE_P1 || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(voiceLfoTarget, VOICE_P1, normalizedMode);
  }
  if (target == VOICE_P2 || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(voiceLfoTarget, VOICE_P2, normalizedMode);
  }
  if (target == VOICE_TRI || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(voiceLfoTarget, VOICE_TRI, LFO_TO_OFF);
  }

  return changed;
}

uint8_t commandTargetFromControlTarget(uint8_t target) {
  if (target == VOICE_P1) return 0;
  if (target == VOICE_P2) return 1;
  if (target == TARGET_GLOBAL) return 3;
  return 255;
}

void queueEnvelopeCommand(uint8_t target, uint8_t param, uint8_t value) {
  uint8_t commandTarget = commandTargetFromControlTarget(target);
  bool idle;
  if (commandTarget == 255) {
    return;
  }

  noInterrupts();
  commandId = (uint8_t)((param << 4) | (commandTarget & 0x0F));
  commandValue = value & 0x7F;
  ++commandGeneration;
  if (commandGeneration == 0) {
    ++commandGeneration;
  }
  commandRepeatsRemaining = CONTROL_COMMAND_REPEATS;
  commandActive = true;
  idle = !transferActive;
  interrupts();
  if (idle) {
    outputIdleEventBit();
  }
}

uint8_t lfoNibbleFromMidi(uint8_t value, uint8_t deadzone) {
  if (value < deadzone) {
    return 0;
  }
  return (uint8_t)((((unsigned int)(value - deadzone)) * 15U + ((127U - deadzone) / 2U)) / (127U - deadzone));
}

uint8_t lfoNibbleFromMidiStable(uint8_t value, uint8_t current, uint8_t deadzone, uint8_t margin) {
  uint8_t mapped = lfoNibbleFromMidi(value, deadzone);

  if (mapped == current) {
    return current;
  }
  if (mapped == current + 1) {
    uint8_t boundary = (uint8_t)(((unsigned int)mapped * 127U + 7U) / 15U);
    if (value < boundary + margin) {
      return current;
    }
  } else if (current == mapped + 1) {
    uint8_t boundary = (uint8_t)(((unsigned int)current * 127U + 7U) / 15U);
    if (value + margin > boundary) {
      return current;
    }
  }

  return mapped;
}

uint8_t sustainLevelFromMidi(uint8_t value) {
  if (value < 8) {
    return 0;
  }

  unsigned long x = value - 8;
  unsigned long max = 119UL * 119UL;
  uint8_t level = (uint8_t)((x * x * 15UL + (max / 2UL)) / max);
  if (level > 15) level = 15;
  return level;
}

bool applyControlValue(uint8_t target, uint8_t controlIndex, uint8_t value) {
  bool changed = false;

  if (controlIndex == CTRL_ATTACK) {
    changed = applyEnvelopeControl(target, PARAM_ATTACK, value, envAttack);
  } else if (controlIndex == CTRL_DECAY) {
    changed = applyEnvelopeControl(target, PARAM_DECAY, value, envDecay);
  } else if (controlIndex == CTRL_RELEASE) {
    changed = applyEnvelopeControl(target, PARAM_RELEASE, value, envRelease);
  } else if (controlIndex == CTRL_SUSTAIN) {
    changed = applyEnvelopeControl(target, PARAM_SUSTAIN, value, envSustain);
  } else if (controlIndex == CTRL_DUTY) {
    uint8_t duty = value >> 5;
    if (duty > 3) duty = 3;

    if (target == TARGET_GLOBAL) {
      // Appliquer sur P1 et P2 en mode GLOBAL
      if (pulseDuty[0] != duty) {
        pulseDuty[0] = duty;
        changed = true;
      }
      if (pulseDuty[1] != duty) {
        pulseDuty[1] = duty;
        changed = true;
      }
    } else if (target == VOICE_P1 && pulseDuty[0] != duty) {
      // Appliquer uniquement sur P1
      pulseDuty[0] = duty;
      changed = true;
    } else if (target == VOICE_P2 && pulseDuty[1] != duty) {
      // Appliquer uniquement sur P2
      pulseDuty[1] = duty;
      changed = true;
    }
  } else if (controlIndex == CTRL_LFO_DEPTH) {
    changed = applyLfoControl(target, voiceLfoDepth, value);
  } else if (controlIndex == CTRL_LFO_RATE) {
    changed = applyLfoControl(target, voiceLfoRate, value);
  }

  return changed;
}

bool applyEnvelopeControl(uint8_t target, uint8_t param, uint8_t value, uint8_t* values) {
  bool changed = false;
  (void)param;

  if (target == VOICE_P1 || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(values, VOICE_P1, value);
  }
  if (target == VOICE_P2 || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(values, VOICE_P2, value);
  }

  return changed;
}

bool applyLfoControl(uint8_t target, uint8_t* values, uint8_t value) {
  bool changed = false;
  const uint8_t deadzone = (values == voiceLfoRate) ? 2 : 3;
  const uint8_t margin = 2;

  if (target == VOICE_P1 || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(values, VOICE_P1, lfoNibbleFromMidiStable(value, values[VOICE_P1], deadzone, margin));
  }
  if (target == VOICE_P2 || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(values, VOICE_P2, lfoNibbleFromMidiStable(value, values[VOICE_P2], deadzone, margin));
  }
  if (target == VOICE_TRI || target == TARGET_GLOBAL) {
    changed |= setVoiceValue(values, VOICE_TRI, lfoNibbleFromMidiStable(value, values[VOICE_TRI], deadzone, margin));
  }

  return changed;
}

void forceControlRescan() {
  lastControlValue[CTRL_ATTACK] = 255;
  lastControlValue[CTRL_DECAY] = 255;
  lastControlValue[CTRL_SUSTAIN] = 255;
  lastControlValue[CTRL_RELEASE] = 255;
  lastControlValue[CTRL_DUTY] = 255;
}

bool applyCurrentLfoSnapshotToTarget(uint8_t target) {
  bool changed = false;
  uint8_t depthValue = lastControlValue[CTRL_LFO_DEPTH];
  uint8_t rateValue = lastControlValue[CTRL_LFO_RATE];

  if (target == TARGET_INVALID) {
    return false;
  }

  if (depthValue == 255) {
    depthValue = mapRawToMidi(filteredControlRaw[CTRL_LFO_DEPTH]);
  }
  if (rateValue == 255) {
    rateValue = mapRawToMidi(filteredControlRaw[CTRL_LFO_RATE]);
  }

  changed |= applyLfoControl(target, voiceLfoDepth, depthValue);
  changed |= applyLfoControl(target, voiceLfoRate, rateValue);
  changed |= applyLfoTarget(target, currentLfoTarget);
  return changed;
}

void armControlPickupForTargetChange() {
  const uint8_t controls[7] = {CTRL_ATTACK, CTRL_DECAY, CTRL_SUSTAIN, CTRL_RELEASE, CTRL_DUTY, CTRL_LFO_DEPTH, CTRL_LFO_RATE};

  for (uint8_t i = 0; i < 7; ++i) {
    uint8_t controlIndex = controls[i];
    uint8_t currentValue = mapRawToMidi(filteredControlRaw[controlIndex]);
    controlPickupReference[controlIndex] = currentValue;
    lastControlValue[controlIndex] = currentValue;
    controlPickupArmed[controlIndex] = true;
  }
}

void setupControls() {
  currentControlTarget = VOICE_P1;
  pendingControlTarget = VOICE_P1;
  pendingControlTargetCount = 0;
  setNesD3Bit(0);
  setNesD4Bit(0);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
#if defined(DIDR0)
  DIDR0 |= _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D) | _BV(ADC3D) | _BV(ADC4D) | _BV(ADC5D);
#endif
#if defined(DIDR1) && defined(ADC6D) && defined(ADC7D)
  DIDR1 |= _BV(ADC6D) | _BV(ADC7D);
#endif

  currentControlTarget = readControlTarget();
  pendingControlTarget = currentControlTarget;
  pendingControlTargetCount = 4;
  currentLfoTarget = readLfoTargetSelector();
  pendingLfoTarget = currentLfoTarget;
  pendingLfoTargetCount = 4;

  const uint8_t initControls[7] = {CTRL_ATTACK, CTRL_DECAY, CTRL_SUSTAIN, CTRL_RELEASE, CTRL_DUTY, CTRL_LFO_DEPTH, CTRL_LFO_RATE};
  for (uint8_t i = 0; i < 7; ++i) {
    uint8_t controlIndex = initControls[i];
    filteredControlRaw[controlIndex] = readAnalogLight(CONTROL_PINS[controlIndex]);
    lastControlValue[controlIndex] = mapRawToMidi(filteredControlRaw[controlIndex]);
    controlPickupReference[controlIndex] = lastControlValue[controlIndex];
    controlPickupArmed[controlIndex] = false;
  }

  if (FORCE_D3_DIAG_PRESET) {
    // Temporary diagnostic preset: if D3 is really received by the NES,
    // P1 and P2 should sound obviously different even before touching pots.
    envAttack[VOICE_P1] = 0;
    envDecay[VOICE_P1] = 96;
    envSustain[VOICE_P1] = 40;
    envRelease[VOICE_P1] = 96;
    pulseDuty[0] = 0;

    envAttack[VOICE_P2] = 0;
    envDecay[VOICE_P2] = 0;
    envSustain[VOICE_P2] = 127;
    envRelease[VOICE_P2] = 0;
    pulseDuty[1] = 3;
  } else {
    // Force a neutral known-safe D3 state at boot.
    // Controls take over only after a real slider move or a selector change.
    envAttack[VOICE_P1] = 0;
    envDecay[VOICE_P1] = 0;
    envSustain[VOICE_P1] = 127;
    envRelease[VOICE_P1] = 0;
    pulseDuty[0] = 2;

    envAttack[VOICE_P2] = 0;
    envDecay[VOICE_P2] = 0;
    envSustain[VOICE_P2] = 127;
    envRelease[VOICE_P2] = 0;
    pulseDuty[1] = 2;
  }

  if (FORCE_D4_DIAG_PRESET) {
    // LFO V2 Phase 1: seul AMP est supporté
    voiceLfoDepth[VOICE_P1] = 15;
    voiceLfoRate[VOICE_P1] = 15;
    voiceLfoTarget[VOICE_P1] = LFO_TO_AMP;

    voiceLfoDepth[VOICE_P2] = 15;
    voiceLfoRate[VOICE_P2] = 15;
    voiceLfoTarget[VOICE_P2] = LFO_TO_AMP;

    voiceLfoDepth[VOICE_TRI] = 12;
    voiceLfoRate[VOICE_TRI] = 15;
    voiceLfoTarget[VOICE_TRI] = LFO_TO_AMP;
  } else if (currentControlTarget != TARGET_INVALID) {
    applyLfoControl(currentControlTarget, voiceLfoDepth, lastControlValue[CTRL_LFO_DEPTH]);
    applyLfoControl(currentControlTarget, voiceLfoRate, lastControlValue[CTRL_LFO_RATE]);
  }

  applyCurrentLfoSnapshotToTarget(currentControlTarget);
  rebuildPayload();
}

void processControls() {
  const uint8_t controls[7] = {CTRL_ATTACK, CTRL_DECAY, CTRL_SUSTAIN, CTRL_RELEASE, CTRL_DUTY, CTRL_LFO_DEPTH, CTRL_LFO_RATE};
  unsigned long nowMs = millis();
  bool changed = false;
  uint8_t selectedTarget;
  uint8_t selectedLfoTarget;

  if ((unsigned long)(nowMs - lastControlScanMs) < CONTROL_SCAN_MS) {
    return;
  }
  lastControlScanMs = nowMs;

  selectedTarget = readControlTarget();

  if (selectedTarget == TARGET_INVALID) {
    pendingControlTargetCount = 0;
    return;
  }

  if (selectedTarget != pendingControlTarget) {
    pendingControlTarget = selectedTarget;
    pendingControlTargetCount = 1;
  } else if (pendingControlTargetCount < 255) {
    ++pendingControlTargetCount;
  }

  if (pendingControlTargetCount >= 4 && currentControlTarget != pendingControlTarget) {
    currentControlTarget = pendingControlTarget;
    armControlPickupForTargetChange();
    changed |= applyLfoTarget(currentControlTarget, currentLfoTarget);
  }

  selectedLfoTarget = readLfoTargetSelector();
  if (selectedLfoTarget != pendingLfoTarget) {
    pendingLfoTarget = selectedLfoTarget;
    pendingLfoTargetCount = 1;
  } else if (pendingLfoTargetCount < 255) {
    ++pendingLfoTargetCount;
  }

  if (pendingLfoTargetCount >= 4 && currentLfoTarget != pendingLfoTarget) {
    currentLfoTarget = pendingLfoTarget;
    changed |= applyLfoTarget(currentControlTarget, currentLfoTarget);
  }
  
  for (uint8_t i = 0; i < 7; ++i) {
    uint8_t controlIndex = controls[i];
    int raw = readAnalogLight(CONTROL_PINS[controlIndex]);
    bool isLfoControl = (controlIndex == CTRL_LFO_DEPTH || controlIndex == CTRL_LFO_RATE);
    filteredControlRaw[controlIndex] = (filteredControlRaw[controlIndex] * 3 + raw + 2) / 4;

    uint8_t value = mapRawToMidi(filteredControlRaw[controlIndex]);
    if (controlPickupArmed[controlIndex]) {
      if (abs((int)value - (int)controlPickupReference[controlIndex]) < 8) {
        continue;
      }
      controlPickupArmed[controlIndex] = false;
    }

    if (lastControlValue[controlIndex] != 255 &&
        abs((int)value - (int)lastControlValue[controlIndex]) <
          (isLfoControl ? 2 : 5)) {
      continue;
    }

    if (!isLfoControl &&
        (unsigned long)(nowMs - lastD3ControlSendMs) < D3_CONTROL_MIN_SEND_MS) {
      continue;
    }

    lastControlValue[controlIndex] = value;
    changed |= applyControlValue(currentControlTarget, controlIndex, value);
    if (!isLfoControl) {
      lastD3ControlSendMs = nowMs;
    }
  }

  if (changed) {
    rebuildPayload();
  }
}

void removeGlobalNote(uint8_t note) {
  for (uint8_t i = 0; i < globalStackCount; ++i) {
    if (globalStack[i].note == note) {
      for (uint8_t j = i; (uint8_t)(j + 1) < globalStackCount; ++j) {
        globalStack[j] = globalStack[j + 1];
      }
      --globalStackCount;
      return;
    }
  }
}

void noteOn(uint8_t channel, uint8_t note, uint8_t vel) {
  if (MIDI_DIAG_ANY_CHANNEL_TO_P2) {
    setP2Live(note, vel, true);
    return;
  }

  if (channel == CH_GLOBAL) {
    addGlobalNote(note, vel);
    refreshGlobalVoices();
    return;
  }

  if (channel >= CH_P1 && channel <= CH_NOISE) {
    uint8_t voiceId = voiceFromChannel(channel);
    setVoice(voiceId, note, vel, true);
  }
}

void noteOff(uint8_t channel, uint8_t note) {
  if (MIDI_DIAG_ANY_CHANNEL_TO_P2) {
    if ((p2OutNote & 0x7F) == (note & 0x7F)) {
      setP2Live(note, 0, false);
    }
    return;
  }

  if (channel == CH_GLOBAL) {
    removeGlobalNote(note);
    refreshGlobalVoices();
    return;
  }

  if (channel >= CH_P1 && channel <= CH_NOISE) {
    uint8_t voiceId = voiceFromChannel(channel);
    releaseVoice(voiceId, note);
  }
}

bool isChannelStatus(uint8_t status) {
  return (status >= 0x80 && status <= 0xEF);
}

uint8_t expectedDataBytes(uint8_t status) {
  uint8_t message = status & 0xF0;
  if (message == 0xC0 || message == 0xD0) return 1;
  if (message >= 0x80 && message <= 0xE0) return 2;
  return 0;
}

void handleChannelMessage(uint8_t status, uint8_t data0, uint8_t data1) {
  uint8_t message = status & 0xF0;
  uint8_t channel = (status & 0x0F) + 1;

  if (message == 0x90 && data1 != 0) {
    noteOn(channel, data0 & 0x7F, data1 & 0x7F);
  } else if (message == 0x80 || message == 0x90) {
    noteOff(channel, data0 & 0x7F);
  }
}

void midiRawByteToP2(uint8_t b) {
  if (b >= 0xF8) {
    return;
  }

  midiDiagStep = (midiDiagStep + 1) & 3;
  setVoice(VOICE_P2, 48 + midiDiagStep * 7, 120, true);
}

void processMidiByte(uint8_t b) {
  if (b >= 0xF8) {
    handleMidiRealtime(b);
    return;
  }

  if (b & 0x80) {
    if (!isChannelStatus(b) && (b == 0xF2 || b == 0xF3 || b == 0xF6)) {
      softenSyncedArpClock(b == 0xF2);
    }
    runningStatus = isChannelStatus(b) ? b : 0;
    dataIndex = 0;
    return;
  }

  if (runningStatus == 0) {
    return;
  }

  dataBytes[dataIndex++] = b & 0x7F;
  if (dataIndex >= expectedDataBytes(runningStatus)) {
    if (expectedDataBytes(runningStatus) == 2) {
      handleChannelMessage(runningStatus, dataBytes[0], dataBytes[1]);
    }
    dataIndex = 0;
  }
}

void handleMidiRealtime(uint8_t b) {
  (void)b;
}

void setupNesPort() {
  pinMode(NES_LATCH_PIN, INPUT_PULLUP);
  pinMode(NES_DATA_PIN, OUTPUT);
  outputIdleEventBit();

  nesLatchHigh = (PINB & _BV(PB4)) != 0;
  sawOutRise = false;
  outRiseTicks = 0;

  noInterrupts();
  PCICR |= _BV(PCIE0);
  PCMSK0 |= _BV(PCINT4);
  PCIFR |= _BV(PCIF0);
  interrupts();
}

void setup() {
  pinMode(0, INPUT_PULLUP);
  Serial.begin(MIDI_BAUD);
  setupTimingTimer();
  setupNesPort();
  setupControls();
  noInterrupts();
  writeP2PacketFromState();
  writeD3PacketFromState();
  writeD4PacketFromState();
  // Arm a repeated all-off packet so hot-plugging after ROM launch syncs cleanly.
  packetGeneration = 0;
  sentGeneration = 0;
  transferGeneration = 0;
  repeatGeneration = 0;
  repeatsRemaining = 0;
  transferActive = false;
  shiftByteIndex = PACKET_SIZE;
  markCurrentPacketDirty(NES_STARTUP_REPEATS);
  interrupts();
  outputIdleEventBit();
}

void loop() {
  if (FORCE_D4_STATIC_HIGH_TEST) {
    setNesDataBit(0);
    setNesD3Bit(0);
    setNesD4Bit(1);
    delay(10);
    return;
  }

  while (Serial.available() > 0) {
    uint8_t b = (uint8_t)Serial.read();
    if (MIDI_DIAG_RAW_BYTE_TO_P2) {
      midiRawByteToP2(b);
    } else {
      processMidiByte(b);
    }
  }

  serviceHeldNoteOffs();
  serviceArpeggiator();
  processControls();
}
