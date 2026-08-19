#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define P1_VOL       (*(volatile unsigned char*)0x4000)
#define P1_SWEEP     (*(volatile unsigned char*)0x4001)
#define P1_LO        (*(volatile unsigned char*)0x4002)
#define P1_HI        (*(volatile unsigned char*)0x4003)
#define P2_VOL       (*(volatile unsigned char*)0x4004)
#define P2_SWEEP     (*(volatile unsigned char*)0x4005)
#define P2_LO        (*(volatile unsigned char*)0x4006)
#define P2_HI        (*(volatile unsigned char*)0x4007)
#define TRI_LINEAR   (*(volatile unsigned char*)0x4008)
#define TRI_LO       (*(volatile unsigned char*)0x400A)
#define TRI_HI       (*(volatile unsigned char*)0x400B)
#define NOI_VOL      (*(volatile unsigned char*)0x400C)
#define NOI_LO       (*(volatile unsigned char*)0x400E)
#define NOI_HI       (*(volatile unsigned char*)0x400F)
#define DMC_FREQ     (*(volatile unsigned char*)0x4010)
#define DMC_RAW      (*(volatile unsigned char*)0x4011)
#define DMC_START    (*(volatile unsigned char*)0x4012)
#define DMC_LEN      (*(volatile unsigned char*)0x4013)
#define APU_FRAME    (*(volatile unsigned char*)0x4017)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

#define REG_DUTY     0x00
#define REG_ADSR_A   0x01
#define REG_ADSR_D   0x02
#define REG_ADSR_S   0x03
#define REG_ADSR_R   0x04
#define REG_TRI_NOTE 0x05
#define REG_TRI_GATE 0x06
#define REG_TRI_TRIG 0x07
#define REG_P1_NOTE  0x08
#define REG_P1_GATE  0x09
#define REG_P1_TRIG  0x0A
#define REG_PULSE_FRAME REG_P1_TRIG
#define REG_P2_NOTE  0x0B
#define REG_P2_GATE  0x0C
#define REG_P2_TRIG  0x0D
#define REG_NOI_NOTE 0x0E
#define REG_NOI_GATE 0x0F

#define V2_IDLE          0x00
#define V2_READ_STATUS   0x01
#define V2_READ_DATA     0x02

#define V2_STATUS_P1_PENDING 0x01
#define V2_STATUS_P2_PENDING 0x02
#define V2_STATUS_TRI_PENDING 0x04
#define V2_STATUS_NOISE_PENDING 0x08
#define V2_STATUS_EVENT_PENDING 0x10

#define V2_VOICE_P1   0x00
#define V2_VOICE_P2   0x01
#define V2_VOICE_TRI  0x02
#define V2_VOICE_NOISE 0x03
#define V2_VOICE_DMC  0x04
#define V2_VOICE_CTRL 0x05

#define CTRL_ATTACK     0x00
#define CTRL_DECAY      0x01
#define CTRL_VOLUME     0x02
#define CTRL_RELEASE    0x03
#define CTRL_DUTY       0x04
#define CTRL_LFO_DEPTH  0x05
#define CTRL_LFO_RATE   0x06
#define CTRL_LFO_DUTY_DEPTH 0x07
#define CTRL_LFO_AMP_DEPTH  0x08
#define CTRL_LFO_DELAY      0x09
#define CTRL_LFO_WAVE       0x0A
#define CTRL_DMC_TRIGGER    0x0B
#define CTRL_MODE_FLAGS     0x0C

#define VOICE_CTRL_META_BASE  0xF0
#define VOICE_CTRL_VALUE_BASE 0xE0
#define DMC_CTRL_META_BASE    0x70
#define DMC_CTRL_VALUE_BASE   0x60
#define DMC_TRIGGER_META_LO   0xFD
#define DMC_TRIGGER_META_HI   0xFE
#define DMC_TRIGGER_VALUE_BASE 0xE0
#define DMC_SAFE_RATE_MAX     0x0D
#define VOICE_BEND_META_BASE  0xA0
#define VOICE_BEND_VALUE_BASE 0xB0
#define VOICE_GLIDE_META_BASE 0xC0
#define VOICE_GLIDE_VALUE_BASE 0xD0

#define DIRTY_P1    0x01
#define DIRTY_P2    0x02
#define DIRTY_TRI   0x04
#define DIRTY_NOISE 0x08

#define ENV_OFF     0
#define ENV_ATTACK  1
#define ENV_DECAY   2
#define ENV_SUSTAIN 3
#define ENV_RELEASE 4

#define GLIDE_TICK_DIV 1
#define P1_GLIDE_STEP 24
#define P2_GLIDE_STEP 24
#define TRI_GLIDE_STEP 96
#define MIDI_DIRECT_PULSE 0
#define BUS_READ_DELAY 1
#define BUS_IDLE_DELAY 1
#define BUS_ACK_DELAY 1

#define LFO_WAVE_TRIANGLE 0
#define LFO_WAVE_SAW      1
#define LFO_WAVE_SINE     2
#define LFO_WAVE_SQUARE   3

typedef struct Voice {
  unsigned char note;
  unsigned char gate;
  unsigned char trig;
  unsigned char last_note;
  unsigned char last_gate;
  unsigned char last_trig;
  unsigned char phase;
  unsigned char level;
  unsigned char counter;
} Voice;

typedef struct PulsePitch {
  unsigned int base_timer;
  unsigned int current_timer;
  unsigned int current_note_cs;
  unsigned int target_note_cs;
  signed char lfo_offset;
  signed char bend_offset;
  signed char bend_raw;
  signed char detune_offset;
  unsigned char glide_step;
  unsigned char glide_div;
  unsigned char written_lo;
  unsigned char written_hi;
} PulsePitch;

typedef struct DmcSample {
  unsigned char rate;
  unsigned char start;
  unsigned char length;
  unsigned char level;
} DmcSample;

typedef struct VoiceControl {
  unsigned char attack;
  unsigned char decay;
  unsigned char volume;
  unsigned char release;
  unsigned char duty;
  unsigned char lfo_pitch_depth;
  unsigned char lfo_rate;
  unsigned char lfo_duty_depth;
  unsigned char lfo_amp_depth;
  unsigned char lfo_delay;
  unsigned char lfo_wave;
  unsigned char sample_pitch;
  unsigned char mode_flags;
} VoiceControl;

typedef struct LfoState {
  unsigned char phase;
  unsigned char div;
  unsigned char delay_counter;
} LfoState;

static const unsigned int timer_table[] = {
  1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906,
  856,808,762,720,678,640,604,570,538,508,480,453,
  428,404,381,360,339,320,302,285,269,254,240,226,
  214,202,190,180,170,160,151,143,135,127,120,113,
  107,101,95,90,85,80,75,71,67,63,60,56,
  53,50,47,45,42,40,37,35,33,31,30,28
};

static const unsigned char noise_period_table[16] = {
  0x0F,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,0x08,
  0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00
};

static const signed char sine_wave[32] = {
   0,  3,  6,  9, 11, 13, 14, 15,
  16, 15, 14, 13, 11,  9,  6,  3,
   0, -3, -6, -9,-11,-13,-14,-15,
 -16,-15,-14,-13,-11, -9, -6, -3
};

static const signed char triangle_wave[32] = {
  -16,-14,-12,-10,-8,-6,-4,-2,
    0,  2,  4,  6,  8,10,12,14,
   16, 14, 12, 10, 8, 6, 4, 2,
    0, -2, -4, -6, -8,-10,-12,-14
};

#include "dmc_samples.h"

static Voice p1 = {40,0,0,0,0,0xFF,ENV_OFF,0,0};
static Voice p2 = {52,0,0,0,0,0xFF,ENV_OFF,0,0};
static Voice noi = {24,0,0,0,0,0xFF,ENV_OFF,0,0};
static unsigned char tri_note = 40;
static unsigned char tri_gate = 0;
static unsigned char tri_trig = 0;
static unsigned char tri_last_note = 0;
static unsigned char tri_last_gate = 0;
static unsigned char tri_last_trig = 0xFF;
static VoiceControl p1_ctrl = {0,0,15,0,2,0,0,0,0,0,LFO_WAVE_SINE,8,0};
static VoiceControl p2_ctrl = {0,0,15,0,2,0,0,0,0,0,LFO_WAVE_SINE,8,0};
static VoiceControl tri_ctrl = {0,0,15,0,0,0,0,0,0,0,LFO_WAVE_SINE,8,0};
static VoiceControl noise_ctrl = {0,0,15,0,8,0,0,0,0,0,LFO_WAVE_SINE,8,0};
static VoiceControl dmc_ctrl = {0,0,15,0,0,0,0,0,0,0,LFO_WAVE_SINE,8,0};
static unsigned char duty = 2;
static unsigned char env_a = 0;
static unsigned char env_d = 0;
static unsigned char env_s = 15;
static unsigned char env_r = 0;
static PulsePitch p1_pitch = {0,0,0,0,0,0,0,0,P1_GLIDE_STEP,0,0xFF,0xFF};
static PulsePitch p2_pitch = {0,0,0,0,0,0,0,0,P2_GLIDE_STEP,0,0xFF,0xFF};
static unsigned char lfo_depth = 0;
static unsigned char lfo_rate = 0;
static unsigned char noise_mode = 0xFF;
static PulsePitch tri_pitch = {0,0,0,0,0,0,0,0,TRI_GLIDE_STEP,0,0xFF,0xFF};
static LfoState p1_lfo = {0,0,0};
static LfoState p2_lfo = {128,0,0};
static LfoState tri_lfo = {64,0,0};
static LfoState noise_lfo = {32,0,0};
static unsigned char dmc_enable_mask = 0;
static signed char p1_duty_lfo_offset = 0;
static signed char p2_duty_lfo_offset = 0;
static signed char noise_timbre_lfo_offset = 0;
static signed char p1_amp_lfo_offset = 0;
static signed char p2_amp_lfo_offset = 0;
static signed char noise_amp_lfo_offset = 0;
static unsigned char p1_ctrl_pending = 0xFF;
static unsigned char p2_ctrl_pending = 0xFF;
static unsigned char tri_ctrl_pending = 0xFF;
static unsigned char noise_ctrl_pending = 0xFF;
static unsigned char dmc_ctrl_pending = 0xFF;
static unsigned char dmc_trigger_bank = 0;
static unsigned char p1_bend_pending = 0xFF;
static unsigned char p2_bend_pending = 0xFF;
static unsigned char tri_bend_pending = 0xFF;
static unsigned char p1_glide_pending = 0xFF;
static unsigned char p2_glide_pending = 0xFF;
static unsigned char tri_glide_pending = 0xFF;

static void update_musical_vibrato(void);
static unsigned char transport_needs_pitch_tick(void);

static void delay_short(unsigned char outer) {
  volatile unsigned char i;
  volatile unsigned char j;
  for (i = 0; i < outer; ++i) {
    for (j = 0; j < 255; ++j) {
    }
  }
}

static unsigned int timer_from_note(unsigned char note) {
  if (note < 24) note = 24;
  if (note > 95) note = 95;
  return timer_table[note - 24];
}

static unsigned int note_to_cs(unsigned char note) {
  return (unsigned int)note << 6;
}

static unsigned int timer_from_note_cs(unsigned int cs) {
  unsigned char note;
  unsigned char frac;
  unsigned int timer_a;
  unsigned int timer_b;

  note = (unsigned char)(cs >> 6);
  if (note < 24U) note = 24U;
  if (note > 95U) note = 95U;
  frac = (unsigned char)(cs & 0x3FU);
  timer_a = timer_from_note(note);
  if (frac == 0U) {
    return timer_a;
  }
  timer_b = timer_from_note((unsigned char)((note < 95U) ? (note + 1U) : 95U));
  return (unsigned int)((((unsigned long)timer_a * (unsigned long)(64U - frac)) +
                         ((unsigned long)timer_b * (unsigned long)frac) + 32UL) / 64UL);
}

static unsigned char bus_encode(unsigned char opcode) {
  return (unsigned char)(((opcode & 0x01) << 2) | (opcode & 0x02) | ((opcode & 0x04) >> 2));
}

static unsigned char read_phase(unsigned char opcode) {
  JOY_STROBE = bus_encode(opcode);
  delay_short(BUS_READ_DELAY);
  return (unsigned char)(JOY2_PORT & 0x1F);
}

static void write_idle(void) {
  JOY_STROBE = bus_encode(V2_IDLE);
  delay_short(BUS_IDLE_DELAY);
}

static void ack_phase(unsigned char opcode) {
  JOY_STROBE = bus_encode(opcode);
  delay_short(BUS_ACK_DELAY);
  write_idle();
}

static void unlock_transport(void) {
  unsigned char k;
  for (k = 0; k != 4; ++k) {
    JOY_STROBE = bus_encode(0x04); delay_short(180);
    JOY_STROBE = bus_encode(0x02); delay_short(180);
    JOY_STROBE = bus_encode(0x01); delay_short(180);
    JOY_STROBE = bus_encode(0x07); delay_short(180);
  }
  write_idle();
}

static unsigned char ctrl_to_nibble(unsigned char value) {
  return (unsigned char)((value <= 0x0F) ? value : (value >> 3));
}

static unsigned char env_attack_duration(unsigned char attack) {
  if (attack == 0) return 0;
  if (attack <= 2U) return 0;
  return (unsigned char)((attack - 1U) >> 1);
}

static unsigned char env_decay_is_sustain(unsigned char decay) {
  return (unsigned char)(decay >= 15U);
}

static unsigned char env_decay_duration(unsigned char decay) {
  static const unsigned char decay_table[15] = {
    1,1,1,2,2,3,4,5,7,9,12,16,22,30,42
  };
  if (env_decay_is_sustain(decay)) return 0;
  return decay_table[decay];
}

static unsigned char env_release_duration(unsigned char release, unsigned char level) {
  unsigned int duration;
  if (release == 0 || level == 0) return 0;
  duration = (unsigned int)(4U * release) / level;
  if (duration == 0) duration = 1;
  if (duration > 255U) duration = 255U;
  return (unsigned char)duration;
}

static void restart_env(Voice* v, const VoiceControl* ctrl) {
#if MIDI_DIRECT_PULSE
  v->phase = ENV_SUSTAIN;
  v->level = 15;
  v->counter = 0;
#else
  if (ctrl->attack == 0) {
    v->level = 15;
    if (env_decay_is_sustain(ctrl->decay)) {
      v->phase = ENV_SUSTAIN;
    } else {
      v->phase = ENV_DECAY;
    }
  } else {
    v->phase = ENV_ATTACK;
    v->level = 0;
  }
  v->counter = 0;
#endif
}

static void release_env(Voice* v, const VoiceControl* ctrl) {
#if MIDI_DIRECT_PULSE
  v->phase = ENV_OFF;
  v->level = 0;
  v->counter = 0;
#else
  if (ctrl->release == 0) {
    v->phase = ENV_OFF;
    v->level = 0;
    v->counter = 0;
  } else if (v->phase != ENV_OFF) {
    v->phase = ENV_RELEASE;
    v->counter = 0;
  }
#endif
}

static void tick_env(Voice* v, const VoiceControl* ctrl) {
  unsigned char rate;
  switch (v->phase) {
    case ENV_ATTACK:
      rate = env_attack_duration(ctrl->attack);
      if (rate == 0) {
        v->level = 15;
        v->phase = env_decay_is_sustain(ctrl->decay) ? ENV_SUSTAIN : ENV_DECAY;
        break;
      }
      if (++v->counter >= rate) {
        v->counter = 0;
        if (v->level < 15) ++v->level;
        if (v->level >= 15) {
          v->phase = env_decay_is_sustain(ctrl->decay) ? ENV_SUSTAIN : ENV_DECAY;
        }
      }
      break;
    case ENV_DECAY:
      rate = env_decay_duration(ctrl->decay);
      if (rate == 0) {
        v->phase = ENV_SUSTAIN;
        break;
      }
      if (++v->counter >= rate) {
        v->counter = 0;
        if (v->level > 0) --v->level;
        if (v->level == 0) v->phase = ENV_OFF;
      }
      break;
    case ENV_RELEASE:
      rate = env_release_duration(ctrl->release, v->level);
      if (rate == 0) {
        v->level = 0;
        v->phase = ENV_OFF;
        break;
      }
      if (++v->counter >= rate) {
        v->counter = 0;
        if (v->level > 0) --v->level;
        if (v->level == 0) v->phase = ENV_OFF;
      }
      break;
    default:
      break;
  }
}

static unsigned char pulse_volume_level(const VoiceControl* ctrl, unsigned char level) {
  return (unsigned char)(((unsigned int)level * (unsigned int)ctrl->volume + 7U) / 15U);
}

static unsigned char pulse_output_volume(unsigned char ctrl_volume, unsigned char level) {
  return (unsigned char)(((unsigned int)level * (unsigned int)ctrl_volume + 7U) / 15U);
}

static unsigned char clamp_u4(int value) {
  if (value < 0) return 0;
  if (value > 15) return 15;
  return (unsigned char)value;
}

static signed char clamp_s8(int value) {
  if (value < -127) return -127;
  if (value > 127) return 127;
  return (signed char)value;
}

static unsigned char ctrl_to_vibrato_depth(unsigned char v) {
  if (v == 0) return 0;
  return (unsigned char)(1U + (((unsigned int)v * (unsigned int)v) / 32U));
}

static unsigned char ctrl_to_vibrato_phase_step(unsigned char v) {
  if (v >= 15U) return 160U;
  if (v >= 12U) return (unsigned char)(96U + (unsigned char)((v - 12U) * 16U));
  if (v >= 8U) return (unsigned char)(32U + (unsigned char)((v - 8U) * 12U));
  return (unsigned char)(4U + (unsigned char)(v * 4U));
}

static unsigned char lfo_rate_tick_div(unsigned char rate) {
  if (rate == 0) return 0;
  if (rate >= 12U) return 1U;
  if (rate >= 8U) return 2U;
  if (rate >= 4U) return 3U;
  return (unsigned char)(5U - (rate >> 1));
}

static unsigned char ctrl_to_lfo_delay_ticks(unsigned char delay) {
  if (delay == 0) return 0;
  return (unsigned char)(2U + (delay * 3U));
}

static signed char lfo_wave_sample(unsigned char wave, unsigned char phase) {
  unsigned char index = (unsigned char)((phase >> 3) & 0x1FU);
  switch (wave & 0x03U) {
    case LFO_WAVE_TRIANGLE:
      return triangle_wave[index];
    case LFO_WAVE_SAW:
      return (signed char)(((int)index - 16) * 2);
    case LFO_WAVE_SINE:
      return sine_wave[index];
    case LFO_WAVE_SQUARE:
    default:
      return (index < 16U) ? 16 : -16;
  }
}

static signed char lfo_pitch_offset(unsigned char depth_ctrl, signed char sample) {
  unsigned char depth = ctrl_to_vibrato_depth(depth_ctrl);
  int scaled;
  if (depth == 0) return 0;
  scaled = (int)sample * (int)depth;
  if (scaled >= 0) scaled += 8;
  else scaled -= 8;
  return (signed char)(scaled / 16);
}

static signed char lfo_amp_offset(unsigned char depth_ctrl, signed char sample) {
  int scaled;
  if (depth_ctrl == 0) return 0;
  scaled = (int)sample * (int)(depth_ctrl + 1U);
  if (scaled >= 0) scaled += 16;
  else scaled -= 16;
  return (signed char)(scaled / 32);
}

static signed char lfo_duty_offset(unsigned char depth_ctrl, signed char sample) {
  int scaled;
  if (depth_ctrl == 0) return 0;
  scaled = (int)sample * (int)(depth_ctrl + 1U);
  if (scaled > 120) return 1;
  if (scaled < -120) return -1;
  return 0;
}

static unsigned char control_has_lfo_targets(unsigned char voice_code, const VoiceControl* ctrl) {
  switch (voice_code) {
    case V2_VOICE_P1:
    case V2_VOICE_P2:
      return (unsigned char)(ctrl->lfo_pitch_depth != 0 ||
                             ctrl->lfo_duty_depth != 0 ||
                             ctrl->lfo_amp_depth != 0);
    case V2_VOICE_TRI:
      return (unsigned char)(ctrl->lfo_pitch_depth != 0);
    case V2_VOICE_NOISE:
      return (unsigned char)(ctrl->lfo_duty_depth != 0 ||
                             ctrl->lfo_amp_depth != 0);
    default:
      return 0;
  }
}

static void reset_voice_lfo(LfoState* state, const VoiceControl* ctrl) {
  state->phase = 0;
  state->div = 0;
  state->delay_counter = ctrl_to_lfo_delay_ticks(ctrl->lfo_delay);
}

static void update_voice_lfo(unsigned char voice_code,
                             const VoiceControl* ctrl,
                             LfoState* state,
                             unsigned char audible,
                             signed char* pitch_offset,
                             signed char* duty_offset,
                             signed char* amp_offset) {
  unsigned char tick_div;
  unsigned char phase_step;
  signed char sample;

  if (!audible || ctrl->lfo_rate == 0 || !control_has_lfo_targets(voice_code, ctrl)) {
    state->div = 0;
    if (pitch_offset != 0) *pitch_offset = 0;
    if (duty_offset != 0) *duty_offset = 0;
    if (amp_offset != 0) *amp_offset = 0;
    return;
  }

  tick_div = lfo_rate_tick_div(ctrl->lfo_rate);
  phase_step = ctrl_to_vibrato_phase_step(ctrl->lfo_rate);
  ++state->div;
  if (state->div < tick_div) {
    return;
  }
  state->div = 0;

  if (state->delay_counter != 0U) {
    --state->delay_counter;
    if (pitch_offset != 0) *pitch_offset = 0;
    if (duty_offset != 0) *duty_offset = 0;
    if (amp_offset != 0) *amp_offset = 0;
    return;
  }

  state->phase = (unsigned char)(state->phase + phase_step);
  sample = lfo_wave_sample(ctrl->lfo_wave, state->phase);
  if (pitch_offset != 0) *pitch_offset = lfo_pitch_offset(ctrl->lfo_pitch_depth, sample);
  if (duty_offset != 0) *duty_offset = lfo_duty_offset(ctrl->lfo_duty_depth, sample);
  if (amp_offset != 0) *amp_offset = lfo_amp_offset(ctrl->lfo_amp_depth, sample);
}

static int pulse_pitch_offset(PulsePitch* pitch) {
  return (int)pitch->lfo_offset +
         (int)pitch->bend_offset +
         (int)pitch->detune_offset;
}

static unsigned int clamp_pulse_timer(int timer) {
  if (timer < 8) return 8;
  if (timer > 2047) return 2047;
  return (unsigned int)timer;
}

static unsigned int pulse_pitch_source_timer(PulsePitch* pitch) {
  if (pitch->current_timer != 0) return pitch->current_timer;
  return pitch->base_timer;
}

static unsigned int timer_from_note_clamped(int note) {
  if (note < 24) note = 24;
  if (note > 95) note = 95;
  return timer_from_note((unsigned char)note);
}

static signed char bend_timer_offset_from_note(unsigned char note, signed char bend_raw) {
  int direction;
  unsigned int src_timer;
  unsigned int note_a;
  unsigned int note_b;
  unsigned int interp;
  unsigned int total_units;
  unsigned int whole;
  unsigned int frac;

  if (bend_raw == 0) return 0;

  direction = (bend_raw >= 0) ? 1 : -1;
  total_units = (unsigned int)((bend_raw >= 0 ? bend_raw : -bend_raw) * 2);
  whole = total_units / 64U;
  frac = total_units % 64U;
  src_timer = timer_from_note_clamped((int)note);
  note_a = timer_from_note_clamped((int)note + (direction * (int)whole));
  if (frac == 0U) {
    return clamp_s8((int)note_a - (int)src_timer);
  }
  note_b = timer_from_note_clamped((int)note + (direction * ((int)whole + 1)));
  interp = (unsigned int)((((unsigned long)note_a * (unsigned long)(64U - frac)) +
                           ((unsigned long)note_b * (unsigned long)frac) + 32UL) / 64UL);
  return clamp_s8((int)interp - (int)src_timer);
}

static void update_pulse_bend(PulsePitch* pitch, unsigned char note) {
  pitch->bend_offset = bend_timer_offset_from_note(note, pitch->bend_raw);
}

static void update_triangle_bend(void) {
  tri_pitch.bend_offset = bend_timer_offset_from_note(tri_note, tri_pitch.bend_raw);
}

static void tick_pulse_glide(PulsePitch* pitch) {
  unsigned int target_cs;
  unsigned int current_cs;
  unsigned int step_cs;

  target_cs = pitch->target_note_cs;
  current_cs = pitch->current_note_cs;
  if (pitch->glide_step == 0) return;
  if (target_cs == 0 || current_cs == 0 || current_cs == target_cs) return;

  ++pitch->glide_div;
  if (pitch->glide_div < GLIDE_TICK_DIV) return;
  pitch->glide_div = 0;

  step_cs = pitch->glide_step;
  if (current_cs < target_cs) {
    unsigned int distance = (unsigned int)(target_cs - current_cs);
    unsigned int adaptive = (unsigned int)((distance + 3U) >> 2);
    if (adaptive > step_cs) step_cs = adaptive;
  } else {
    unsigned int distance = (unsigned int)(current_cs - target_cs);
    unsigned int adaptive = (unsigned int)((distance + 3U) >> 2);
    if (adaptive > step_cs) step_cs = adaptive;
  }

  if (current_cs < target_cs) {
    if ((target_cs - current_cs) <= step_cs) current_cs = target_cs;
    else current_cs = (unsigned int)(current_cs + step_cs);
  } else {
    if ((current_cs - target_cs) <= step_cs) current_cs = target_cs;
    else current_cs = (unsigned int)(current_cs - step_cs);
  }
  pitch->current_note_cs = current_cs;
  pitch->current_timer = timer_from_note_cs(current_cs);
}

static void prime_pulse_glide(PulsePitch* pitch) {
  unsigned int target_cs;
  unsigned int current_cs;
  unsigned int step_cs;

  target_cs = pitch->target_note_cs;
  current_cs = pitch->current_note_cs;
  step_cs = pitch->glide_step;
  if (target_cs == 0 || current_cs == 0 || step_cs == 0 || current_cs == target_cs) {
    return;
  }

  if (current_cs < target_cs) {
    unsigned int distance = (unsigned int)(target_cs - current_cs);
    unsigned int adaptive = (unsigned int)((distance + 3U) >> 2);
    if (adaptive > step_cs) step_cs = adaptive;
  } else {
    unsigned int distance = (unsigned int)(current_cs - target_cs);
    unsigned int adaptive = (unsigned int)((distance + 3U) >> 2);
    if (adaptive > step_cs) step_cs = adaptive;
  }

  if (current_cs < target_cs) {
    if ((target_cs - current_cs) <= step_cs) current_cs = target_cs;
    else current_cs = (unsigned int)(current_cs + step_cs);
  } else {
    if ((current_cs - target_cs) <= step_cs) current_cs = target_cs;
    else current_cs = (unsigned int)(current_cs - step_cs);
  }

  pitch->current_note_cs = current_cs;
  pitch->current_timer = timer_from_note_cs(current_cs);
}

static void tick_triangle_glide(void) {
  tick_pulse_glide(&tri_pitch);
}

static unsigned char voice_is_audible(const Voice* v) {
  return (unsigned char)(v->gate || v->phase != ENV_OFF || v->level != 0);
}

static void write_pulse_timer(unsigned char which, PulsePitch* pitch, unsigned int timer, unsigned char force) {
  unsigned char hi;
  unsigned char lo;
  unsigned char hi_needs_write;

  lo = (unsigned char)(timer & 0xFF);
  hi = (unsigned char)((timer >> 8) & 0x07);
  hi_needs_write = (unsigned char)(pitch->written_hi == 0xFF || hi != pitch->written_hi);
  if (which == 1) {
    if (force || lo != pitch->written_lo) {
      pitch->written_lo = lo;
      P1_LO = lo;
    }
    if (hi_needs_write) {
      pitch->written_hi = hi;
      P1_HI = hi;
    }
  } else {
    if (force || lo != pitch->written_lo) {
      pitch->written_lo = lo;
      P2_LO = lo;
    }
    if (hi_needs_write) {
      pitch->written_hi = hi;
      P2_HI = hi;
    }
  }
}

static void apply_pulse_pitch(Voice* v, unsigned char which, PulsePitch* pitch, unsigned char force) {
  unsigned int source_timer;
  unsigned int timer;

  source_timer = pulse_pitch_source_timer(pitch);
  if (!voice_is_audible(v) || source_timer == 0) return;

  timer = clamp_pulse_timer((int)source_timer + pulse_pitch_offset(pitch));
  write_pulse_timer(which, pitch, timer, force);
}

static void set_pulse_base_note(Voice* v, unsigned char which, PulsePitch* pitch, unsigned char retrigger) {
  pitch->base_timer = timer_from_note(v->note);
  pitch->target_note_cs = note_to_cs(v->note);
  update_pulse_bend(pitch, v->note);
  if (retrigger || pitch->current_timer == 0) {
    pitch->current_note_cs = pitch->target_note_cs;
    pitch->current_timer = pitch->base_timer;
    pitch->glide_div = 0;
  }
  if (which == 1) {
    P1_SWEEP = 0x08;
  } else {
    P2_SWEEP = 0x08;
  }
  apply_pulse_pitch(v, which, pitch, retrigger);
}

static void write_triangle_timer(unsigned int timer) {
  unsigned char lo;
  unsigned char hi;

  lo = (unsigned char)(timer & 0xFF);
  hi = (unsigned char)((timer >> 8) & 0x07);
  if (lo != tri_pitch.written_lo) {
    tri_pitch.written_lo = lo;
    TRI_LO = lo;
  }
  if (hi != tri_pitch.written_hi) {
    tri_pitch.written_hi = hi;
    TRI_HI = hi;
  }
}

static void update_musical_vibrato(void) {
  update_voice_lfo(V2_VOICE_P1, &p1_ctrl, &p1_lfo, voice_is_audible(&p1),
                   &p1_pitch.lfo_offset, &p1_duty_lfo_offset, &p1_amp_lfo_offset);
  update_voice_lfo(V2_VOICE_P2, &p2_ctrl, &p2_lfo, voice_is_audible(&p2),
                   &p2_pitch.lfo_offset, &p2_duty_lfo_offset, &p2_amp_lfo_offset);
  /* LFO disabled on Triangle to reduce system load */
  /* update_voice_lfo(V2_VOICE_TRI, &tri_ctrl, &tri_lfo, tri_gate, */
  /*                  &tri_pitch.lfo_offset, 0, 0); */
  update_voice_lfo(V2_VOICE_NOISE, &noise_ctrl, &noise_lfo, voice_is_audible(&noi),
                   0, &noise_timbre_lfo_offset, &noise_amp_lfo_offset);

  tick_pulse_glide(&p1_pitch);
  tick_pulse_glide(&p2_pitch);
  /* tick_triangle_glide(); */
  apply_pulse_pitch(&p1, 1, &p1_pitch, 0);
  apply_pulse_pitch(&p2, 2, &p2_pitch, 0);
  if (tri_gate && tri_pitch.base_timer != 0) {
    /* LFO disabled on Triangle to reduce system load */
    write_triangle_timer((unsigned int)clamp_pulse_timer((int)pulse_pitch_source_timer(&tri_pitch) +
                                                         /* (int)tri_pitch.lfo_offset + */
                                                         (int)tri_pitch.bend_offset));
  }
}

static unsigned char transport_needs_pitch_tick(void) {
  if (control_has_lfo_targets(V2_VOICE_P1, &p1_ctrl) && p1_ctrl.lfo_rate != 0) return 1;
  if (control_has_lfo_targets(V2_VOICE_P2, &p2_ctrl) && p2_ctrl.lfo_rate != 0) return 1;
  /* LFO disabled on Triangle to reduce system load */
  /* if (control_has_lfo_targets(V2_VOICE_TRI, &tri_ctrl) && tri_ctrl.lfo_rate != 0 && tri_gate) return 1; */
  if (control_has_lfo_targets(V2_VOICE_NOISE, &noise_ctrl) && noise_ctrl.lfo_rate != 0 && voice_is_audible(&noi)) return 1;
  if (p1_pitch.glide_step != 0 && p1_pitch.current_timer != p1_pitch.base_timer) return 1;
  if (p2_pitch.glide_step != 0 && p2_pitch.current_timer != p2_pitch.base_timer) return 1;
  /* if (tri_pitch.glide_step != 0 && tri_pitch.current_timer != tri_pitch.base_timer) return 1; */
  return 0;
}

static void update_pulse(Voice* v, unsigned char which, const VoiceControl* ctrl, PulsePitch* pitch) {
  unsigned char retrigger;
  unsigned char glide_only;
  unsigned char vol;
  unsigned char duty_bits;
  unsigned char ctrl_volume;
  unsigned char was_audible;
  unsigned char duty_value;
  signed char duty_offset;
  signed char amp_offset;

  was_audible = voice_is_audible(v);
  duty_offset = (which == 1) ? p1_duty_lfo_offset : p2_duty_lfo_offset;
  amp_offset = (which == 1) ? p1_amp_lfo_offset : p2_amp_lfo_offset;

  if (v->gate && (!v->last_gate || v->note != v->last_note || v->trig != v->last_trig)) {
    glide_only = (unsigned char)((which == 1 || which == 2) &&
                                 v->last_gate &&
                                 v->note != v->last_note &&
                                 (ctrl->mode_flags & 0x01U) == 0U);
    retrigger = (unsigned char)(!v->last_gate || (v->trig != v->last_trig && !glide_only));
    if (!was_audible) {
      reset_voice_lfo((which == 1) ? &p1_lfo : &p2_lfo, ctrl);
    }
    if (retrigger) restart_env(v, ctrl);
    if (which == 1) {
      if (retrigger) P1_VOL = 0x30;
      set_pulse_base_note(v, 1, pitch, retrigger);
    } else {
      if (retrigger) P2_VOL = 0x30;
      set_pulse_base_note(v, 2, pitch, retrigger);
    }
    if (glide_only) {
      pitch->base_timer = timer_from_note(v->note);
      pitch->target_note_cs = note_to_cs(v->note);
      update_pulse_bend(pitch, v->note);
      if (pitch->current_timer == 0U) {
        pitch->current_note_cs = pitch->target_note_cs;
        pitch->current_timer = pitch->base_timer;
      }
      prime_pulse_glide(pitch);
      apply_pulse_pitch(v, which, pitch, 0);
      pitch->glide_div = 0;
    }
  } else if (!v->gate && v->last_gate) {
    release_env(v, ctrl);
  }

  tick_env(v, ctrl);
  duty_value = ctrl->duty;
  if (duty_value > 2U) duty_value = 2U;
  if (duty_offset > 0 && duty_value < 2U) ++duty_value;
  if (duty_offset < 0 && duty_value > 0U) --duty_value;
  duty_bits = (unsigned char)((duty_value & 0x03U) << 6);
  ctrl_volume = clamp_u4((int)ctrl->volume + (int)amp_offset);
  vol = (unsigned char)(duty_bits | 0x20 | 0x10 | (pulse_output_volume(ctrl_volume, v->level) & 0x0F));
  if (which == 1) {
    P1_VOL = vol;
  } else {
    P2_VOL = vol;
  }

  v->last_note = v->note;
  v->last_gate = v->gate;
  v->last_trig = v->trig;
}

static void update_triangle(void) {
  unsigned int timer;
  if (tri_gate) {
    if (!tri_last_gate || tri_note != tri_last_note || tri_trig != tri_last_trig) {
      tri_pitch.base_timer = timer_from_note(tri_note) >> 1;
      tri_pitch.target_note_cs = note_to_cs(tri_note);
      update_triangle_bend();
      tri_pitch.current_note_cs = tri_pitch.target_note_cs;
      tri_pitch.current_timer = tri_pitch.base_timer;
      tri_pitch.glide_div = 0;
      TRI_LINEAR = 0x80 | 0x7F;
    } else {
      TRI_LINEAR = 0x80 | 0x7F;
    }
    timer = (unsigned int)clamp_pulse_timer((int)pulse_pitch_source_timer(&tri_pitch) +
                                            (int)tri_pitch.lfo_offset +
                                            (int)tri_pitch.bend_offset);
    write_triangle_timer(timer);
  } else if (tri_last_gate) {
    TRI_LINEAR = 0;
    tri_pitch.base_timer = 0;
    tri_pitch.current_timer = 0;
    tri_pitch.current_note_cs = 0;
    tri_pitch.target_note_cs = 0;
    tri_pitch.lfo_offset = 0;
    tri_pitch.written_lo = 0xFF;
    tri_pitch.written_hi = 0xFF;
  }
  tri_last_note = tri_note;
  tri_last_gate = tri_gate;
  tri_last_trig = tri_trig;
}

static const DmcSample* dmc_sample_from_id(unsigned char sample_id) {
  sample_id = (unsigned char)(sample_id & 0x1FU);
  if (sample_id < DMC_SAMPLE_COUNT) {
    return &dmc_samples[sample_id];
  }
  return 0;
}

static void trigger_dmc(const DmcSample* sample, const VoiceControl* ctrl) {
  unsigned int scaled_level;
  unsigned char rate;
  dmc_enable_mask = 0;
  APU_CTRL = 0x0F;
  rate = (unsigned char)(sample->rate & 0x0FU);
  if (rate > DMC_SAFE_RATE_MAX) rate = DMC_SAFE_RATE_MAX;
  DMC_FREQ = rate;
  scaled_level = ((unsigned int)sample->level * (unsigned int)ctrl->volume + 7U) / 15U;
  if (scaled_level > 0x7FU) scaled_level = 0x7FU;
  DMC_RAW = (unsigned char)scaled_level;
  DMC_START = sample->start;
  DMC_LEN = sample->length;
  dmc_enable_mask = 0x10;
  APU_CTRL = 0x1F;
}

static void trigger_dmc_id(unsigned char sample_id) {
  const DmcSample* sample;

  sample = dmc_sample_from_id(sample_id);
  if (sample != 0) {
    trigger_dmc(sample, &dmc_ctrl);
  }
}

static void update_noise(void) {
  unsigned char changed;
  unsigned char new_mode;
  unsigned char timbre_offset;
  unsigned char noise_period;
  unsigned char new_noise_lo;
  unsigned char ctrl_volume;
  unsigned char was_audible;
  int modulated_period;
  was_audible = voice_is_audible(&noi);
  changed = (unsigned char)(noi.gate && (!noi.last_gate || noi.note != noi.last_note || noi.trig != noi.last_trig));
  new_mode = (unsigned char)((noise_ctrl.duty & 0x08U) ? 0x80U : 0x00U);
  timbre_offset = (unsigned char)(noise_ctrl.duty & 0x07U);
  modulated_period = (int)noise_period_table[noi.note & 0x0F] +
                     (int)timbre_offset +
                     (int)noise_timbre_lfo_offset;
  noise_period = clamp_u4(modulated_period);
  new_noise_lo = (unsigned char)(noise_period | new_mode);
  if (changed) {
    if (!was_audible) {
      reset_voice_lfo(&noise_lfo, &noise_ctrl);
    }
    restart_env(&noi, &noise_ctrl);
    NOI_LO = new_noise_lo;
    NOI_HI = 0x18;
  } else if (!noi.gate && noi.last_gate) {
    release_env(&noi, &noise_ctrl);
  } else if (new_noise_lo != noise_mode && voice_is_audible(&noi)) {
    NOI_LO = new_noise_lo;
  }

  tick_env(&noi, &noise_ctrl);
  ctrl_volume = clamp_u4((int)noise_ctrl.volume + (int)noise_amp_lfo_offset);
  NOI_VOL = (unsigned char)(0x20 | 0x10 | (pulse_output_volume(ctrl_volume, noi.level) & 0x0F));
  noise_mode = new_noise_lo;

  noi.last_note = noi.note;
  noi.last_gate = noi.gate;
  noi.last_trig = noi.trig;
}

static void update_audio(void) {
  update_musical_vibrato();
  update_pulse(&p1, 1, &p1_ctrl, &p1_pitch);
  update_pulse(&p2, 2, &p2_ctrl, &p2_pitch);
  update_triangle();
  update_noise();
}

static void commit_live_edges(unsigned char dirty_mask) {
  if (dirty_mask & DIRTY_P1) {
    update_pulse(&p1, 1, &p1_ctrl, &p1_pitch);
  }
  if (dirty_mask & DIRTY_P2) {
    update_pulse(&p2, 2, &p2_ctrl, &p2_pitch);
  }
  if (dirty_mask & DIRTY_TRI) {
    update_triangle();
  }
  if (dirty_mask & DIRTY_NOISE) {
    update_noise();
  }
}

static void apply_voice_edge(Voice* v, unsigned char value) {
  v->note = (unsigned char)(value & 0x7FU);
  v->gate = (value & 0x80U) ? 1 : 0;
  if (v->gate) ++v->trig;
}

static void apply_triangle_edge(unsigned char value) {
  tri_note = (unsigned char)(value & 0x7FU);
  tri_gate = (value & 0x80U) ? 1 : 0;
  if (tri_gate) ++tri_trig;
}

static void apply_noise_edge(unsigned char value) {
  noi.note = (unsigned char)(value & 0x0FU);
  noi.gate = (value & 0x80U) ? 1 : 0;
  if (noi.gate) ++noi.trig;
}

static void apply_dmc_edge(unsigned char value) {
  trigger_dmc_id(value);
}

static void apply_slot_edge(unsigned char voice_code, unsigned char value) {
  switch (voice_code) {
    case V2_VOICE_P1: apply_voice_edge(&p1, value); break;
    case V2_VOICE_P2: apply_voice_edge(&p2, value); break;
    case V2_VOICE_TRI: apply_triangle_edge(value); break;
    case V2_VOICE_NOISE: apply_noise_edge(value); break;
    case V2_VOICE_DMC: apply_dmc_edge(value); break;
    default: break;
  }
}

static VoiceControl* control_for_voice(unsigned char voice_code) {
  switch (voice_code) {
    case V2_VOICE_P1: return &p1_ctrl;
    case V2_VOICE_P2: return &p2_ctrl;
    case V2_VOICE_TRI: return &tri_ctrl;
    case V2_VOICE_NOISE: return &noise_ctrl;
    case V2_VOICE_DMC: return &dmc_ctrl;
    default: break;
  }
  return 0;
}

static unsigned char* pending_control_for_voice(unsigned char voice_code) {
  switch (voice_code) {
    case V2_VOICE_P1: return &p1_ctrl_pending;
    case V2_VOICE_P2: return &p2_ctrl_pending;
    case V2_VOICE_TRI: return &tri_ctrl_pending;
    case V2_VOICE_NOISE: return &noise_ctrl_pending;
    case V2_VOICE_DMC: return &dmc_ctrl_pending;
    default: break;
  }
  return 0;
}

static unsigned char voice_supports_control(unsigned char voice_code, unsigned char control_index) {
  switch (voice_code) {
    case V2_VOICE_P1:
    case V2_VOICE_P2:
      return (unsigned char)(control_index != CTRL_DMC_TRIGGER);
    case V2_VOICE_TRI:
      return (unsigned char)(control_index == CTRL_LFO_DEPTH ||
                             control_index == CTRL_LFO_RATE ||
                             control_index == CTRL_LFO_DELAY ||
                             control_index == CTRL_LFO_WAVE ||
                             control_index == CTRL_MODE_FLAGS);
    case V2_VOICE_NOISE:
      return (unsigned char)(control_index == CTRL_ATTACK ||
                             control_index == CTRL_DECAY ||
             control_index == CTRL_VOLUME ||
             control_index == CTRL_RELEASE ||
             control_index == CTRL_DUTY ||
             control_index == CTRL_LFO_DUTY_DEPTH ||
             control_index == CTRL_LFO_AMP_DEPTH ||
             control_index == CTRL_LFO_RATE ||
                             control_index == CTRL_LFO_DELAY ||
                             control_index == CTRL_LFO_WAVE ||
                             control_index == CTRL_MODE_FLAGS);
    case V2_VOICE_DMC:
      return (unsigned char)(control_index == CTRL_DMC_TRIGGER);
    default:
      break;
  }
  return 0;
}

static unsigned char aux_lane_voice(unsigned char status) {
  (void)status;
  return V2_VOICE_NOISE;
}

static unsigned char is_voice_ctrl_meta(unsigned char value) {
  return (unsigned char)((value & 0xF0U) == VOICE_CTRL_META_BASE);
}

static unsigned char is_voice_ctrl_value(unsigned char value) {
  return (unsigned char)((value & 0xF0U) == VOICE_CTRL_VALUE_BASE);
}

static unsigned char is_dmc_ctrl_meta(unsigned char value) {
  return (unsigned char)(value == DMC_TRIGGER_META_LO || value == DMC_TRIGGER_META_HI);
}

static unsigned char is_dmc_ctrl_value(unsigned char value) {
  return (unsigned char)(!is_dmc_ctrl_meta(value) &&
                         (value & 0xF0U) == DMC_TRIGGER_VALUE_BASE);
}

static void apply_voice_control(unsigned char voice_code,
                                unsigned char control_index,
                                unsigned char value,
                                unsigned char* dirty_mask) {
  VoiceControl* ctrl;

  ctrl = control_for_voice(voice_code);
  if (ctrl == 0 || !voice_supports_control(voice_code, control_index)) return;

  switch (control_index) {
    case CTRL_ATTACK: ctrl->attack = (unsigned char)(value & 0x0FU); break;
    case CTRL_DECAY: ctrl->decay = (unsigned char)(value & 0x0FU); break;
    case CTRL_VOLUME: ctrl->volume = (unsigned char)(value & 0x0FU); break;
    case CTRL_RELEASE: ctrl->release = (unsigned char)(value & 0x0FU); break;
    case CTRL_DUTY:
      ctrl->duty = (unsigned char)((voice_code == V2_VOICE_NOISE) ? (value & 0x0FU) : (value & 0x03U));
      break;
    case CTRL_LFO_DEPTH: ctrl->lfo_pitch_depth = (unsigned char)(value & 0x0FU); break;
    case CTRL_LFO_RATE: ctrl->lfo_rate = (unsigned char)(value & 0x0FU); break;
    case CTRL_LFO_DUTY_DEPTH: ctrl->lfo_duty_depth = (unsigned char)(value & 0x0FU); break;
    case CTRL_LFO_AMP_DEPTH: ctrl->lfo_amp_depth = (unsigned char)(value & 0x0FU); break;
    case CTRL_LFO_DELAY: ctrl->lfo_delay = (unsigned char)(value & 0x0FU); break;
    case CTRL_LFO_WAVE: ctrl->lfo_wave = (unsigned char)(value & 0x03U); break;
    case CTRL_DMC_TRIGGER:
      if (voice_code == V2_VOICE_DMC) {
        trigger_dmc_id((unsigned char)(value & 0x1FU));
      }
      break;
    case CTRL_MODE_FLAGS: ctrl->mode_flags = (unsigned char)(value & 0x0FU); break;
    default: return;
  }

  switch (voice_code) {
    case V2_VOICE_P1: *dirty_mask |= DIRTY_P1; break;
    case V2_VOICE_P2: *dirty_mask |= DIRTY_P2; break;
    case V2_VOICE_TRI: *dirty_mask |= DIRTY_TRI; break;
    case V2_VOICE_NOISE: *dirty_mask |= DIRTY_NOISE; break;
    default: break;
  }
}

static void apply_lane_value(unsigned char voice_code,
                             unsigned char value,
                             unsigned char* dirty_mask) {
  unsigned char* pending;

  if (voice_code == V2_VOICE_DMC) {
    trigger_dmc_id(value);
    dmc_ctrl_pending = 0xFF;
    dmc_trigger_bank = 0;
    return;
  }

  pending = pending_control_for_voice(voice_code);
  if (pending != 0) {
    if ((voice_code == V2_VOICE_DMC && is_dmc_ctrl_meta(value)) ||
        (voice_code != V2_VOICE_DMC && is_voice_ctrl_meta(value))) {
      *pending = (unsigned char)(value & 0x0FU);
      if (voice_code == V2_VOICE_DMC) {
        *pending = CTRL_DMC_TRIGGER;
        dmc_trigger_bank = (unsigned char)((value == DMC_TRIGGER_META_HI) ? 16U : 0U);
      }
      return;
    }
    if ((voice_code == V2_VOICE_DMC && is_dmc_ctrl_value(value)) ||
        (voice_code != V2_VOICE_DMC && is_voice_ctrl_value(value))) {
      if (*pending <= CTRL_MODE_FLAGS) {
        apply_voice_control(voice_code,
                            *pending,
                            (unsigned char)((voice_code == V2_VOICE_DMC) ? (dmc_trigger_bank | (value & 0x0FU)) : (value & 0x0FU)),
                            dirty_mask);
      }
      *pending = 0xFF;
      if (voice_code == V2_VOICE_DMC) {
        dmc_trigger_bank = 0;
      }
      return;
    }
  }

  apply_slot_edge(voice_code, value);
  switch (voice_code) {
    case V2_VOICE_P1: *dirty_mask |= DIRTY_P1; break;
    case V2_VOICE_P2: *dirty_mask |= DIRTY_P2; break;
    case V2_VOICE_TRI: *dirty_mask |= DIRTY_TRI; break;
    case V2_VOICE_NOISE: *dirty_mask |= DIRTY_NOISE; break;
    default: break;
  }
}

static void apply_pulse_frame(unsigned int value) {
  p1.note = (unsigned char)(value & 0x7F);
  p1.gate = (value & 0x80) ? 1 : 0;
  p2.note = (unsigned char)((value >> 8) & 0x7F);
  p2.gate = (value & 0x8000) ? 1 : 0;
  if (p1.gate && (!p1.last_gate || p1.note != p1.last_note)) ++p1.trig;
  if (p2.gate && (!p2.last_gate || p2.note != p2.last_note)) ++p2.trig;
}

static void apply_reg(unsigned char reg_id, unsigned int value) {
  switch (reg_id & 0x0F) {
    case REG_DUTY:
      duty = (unsigned char)(value & 0x03U);
      lfo_depth = (unsigned char)((value >> 2) & 0x07U);
      lfo_rate = (unsigned char)((value >> 5) & 0x07U);
      break;
    case REG_ADSR_A: env_a = ctrl_to_nibble((unsigned char)value); break;
    case REG_ADSR_D: env_d = ctrl_to_nibble((unsigned char)value); break;
    case REG_ADSR_S: env_s = ctrl_to_nibble((unsigned char)value); break;
    case REG_ADSR_R: env_r = ctrl_to_nibble((unsigned char)value); break;
    case REG_TRI_NOTE: tri_note = (unsigned char)(value & 0x7FU); break;
    case REG_TRI_GATE: tri_gate = value ? 1 : 0; break;
    case REG_TRI_TRIG: tri_trig = (unsigned char)value; break;
    case REG_PULSE_FRAME:
      apply_pulse_frame(value);
      break;
    case REG_P1_NOTE:
      p1.note = (unsigned char)(value & 0x7FU);
      p1.gate = (value & 0x80U) ? 1 : 0;
      if (p1.gate) ++p1.trig;
      break;
    case REG_P1_GATE: p1.gate = value ? 1 : 0; break;
    case REG_P2_NOTE:
      p2.note = (unsigned char)(value & 0x7FU);
      p2.gate = (value & 0x80U) ? 1 : 0;
      if (p2.gate) ++p2.trig;
      break;
    case REG_P2_GATE: p2.gate = value ? 1 : 0; break;
    case REG_P2_TRIG: p2.trig = (unsigned char)value; break;
    case REG_NOI_NOTE: noi.note = (unsigned char)(value & 0x0FU); ++noi.trig; break;
    case REG_NOI_GATE: noi.gate = value ? 1 : 0; break;
    default: break;
  }
}

static unsigned char read_stream_field(void) {
  unsigned char field;
  field = read_phase(V2_READ_DATA);
  write_idle();
  return field;
}

static unsigned char read_stream_value(unsigned char* out_value) {
  unsigned char lo_field;
  unsigned char hi_field;

  lo_field = read_stream_field();
  hi_field = read_stream_field();
  *out_value = (unsigned char)(((hi_field & 0x07) << 5) | (lo_field & 0x1F));
  return 1;
}

static unsigned char service_one_event(void) {
  unsigned char status;
  unsigned char p1_pending;
  unsigned char p2_pending;
  unsigned char tri_pending;
  unsigned char noise_pending;
  unsigned char event_pending;
  unsigned char p1_value;
  unsigned char p2_value;
  unsigned char tri_value;
  unsigned char noise_field;
  unsigned char noise_value;
  unsigned char event_header;
  unsigned char event_voice;
  unsigned char event_value;
  unsigned char processed = 0;
  unsigned char dirty_mask = 0;

  status = read_phase(V2_READ_STATUS);
  write_idle();

  p1_pending = (status & V2_STATUS_P1_PENDING) ? 1 : 0;
  p2_pending = (status & V2_STATUS_P2_PENDING) ? 1 : 0;
  tri_pending = (status & V2_STATUS_TRI_PENDING) ? 1 : 0;
  noise_pending = (status & V2_STATUS_NOISE_PENDING) ? 1 : 0;
  event_pending = (status & V2_STATUS_EVENT_PENDING) ? 1 : 0;
  if (!p1_pending && !p2_pending && !tri_pending && !noise_pending && !event_pending) return 0;

  if (p1_pending && !read_stream_value(&p1_value)) {
    return 0;
  }
  if (p2_pending && !read_stream_value(&p2_value)) {
    return 0;
  }
  if (tri_pending && !read_stream_value(&tri_value)) {
    return 0;
  }
  if (noise_pending) {
    noise_field = read_stream_field();
    noise_value = (unsigned char)((noise_field & 0x0FU) | ((noise_field & 0x10U) ? 0x80U : 0x00U));
  }
  if (event_pending) {
    event_header = read_stream_field();
    event_voice = (unsigned char)(event_header & 0x07U);
    if (!read_stream_value(&event_value)) {
      return 0;
    }
  }

  if (p1_pending) {
    apply_lane_value(V2_VOICE_P1, p1_value, &dirty_mask);
    ++processed;
  }
  if (p2_pending) {
    apply_lane_value(V2_VOICE_P2, p2_value, &dirty_mask);
    ++processed;
  }
  if (tri_pending) {
    apply_lane_value(V2_VOICE_TRI, tri_value, &dirty_mask);
    ++processed;
  }
  if (noise_pending) {
    apply_lane_value(V2_VOICE_NOISE, noise_value, &dirty_mask);
    ++processed;
  }
  if (event_pending && event_voice <= V2_VOICE_DMC) {
    apply_lane_value(event_voice, event_value, &dirty_mask);
    ++processed;
  }
  if (processed != 0) {
    commit_live_edges(dirty_mask);
  }
  return processed;
}

#define SERVICE_BATCH_BUDGET 64
#define SERVICE_AUDIO_SLICE 2

static unsigned char service_control_events(void) {
  unsigned char remaining = SERVICE_BATCH_BUDGET;
  unsigned char processed = 0;
  while (remaining != 0) {
    if (!service_one_event()) break;
    ++processed;
    --remaining;
    if ((processed & (SERVICE_AUDIO_SLICE - 1)) == 0) {
      update_audio();
    }
  }
  if (processed != 0) {
    update_audio();
  }
  return processed;
}

int main(void) {
  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x0F;
  DMC_RAW = 0x40;
  P1_VOL = 0x30;
  P2_VOL = 0x30;
  TRI_LINEAR = 0;
  NOI_VOL = 0x30;
  NOI_LO = 0x0F;

  unlock_transport();

  for (;;) {
    if (service_control_events() == 0) {
      update_audio();
      delay_short(1);
    }
  }

  return 0;
}
