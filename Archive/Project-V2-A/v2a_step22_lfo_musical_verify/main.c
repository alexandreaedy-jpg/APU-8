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
#define REG_P2_NOTE  0x0B
#define REG_P2_GATE  0x0C
#define REG_P2_TRIG  0x0D
#define REG_NOI_NOTE 0x0E
#define REG_NOI_GATE 0x0F

#define V2_IDLE          0x00
#define V2_READ_STATUS   0x01
#define V2_READ_REG_ID   0x02
#define V2_READ_VALUE_LO 0x06
#define V2_READ_VALUE_HI 0x04
#define V2_ACK_AND_NEXT  0x05

#define ENV_OFF     0
#define ENV_ATTACK  1
#define ENV_DECAY   2
#define ENV_SUSTAIN 3
#define ENV_RELEASE 4

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

typedef struct DmcSample {
  unsigned char rate;
  unsigned char start;
  unsigned char length;
  unsigned char level;
} DmcSample;

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

static const signed char vibrato_shape[32] = {
   0,  3,  6,  9, 11, 13, 14, 15,
  16, 15, 14, 13, 11,  9,  6,  3,
   0, -3, -6, -9,-11,-13,-14,-15,
 -16,-15,-14,-13,-11, -9, -6, -3
};

static const DmcSample dmc_kick = {0x0C, 0xC0, 0x34, 0x44};
static const DmcSample dmc_snare = {0x0C, 0xCE, 0x1C, 0x42};
static const DmcSample dmc_rim = {0x0C, 0xD6, 0x0F, 0x3A};
static const DmcSample dmc_voice = {0x0C, 0xDA, 0x94, 0x34};

static Voice p1 = {52,1,1,0,0,0xFF,ENV_OFF,0,0};
static Voice p2 = {64,1,1,0,0,0xFF,ENV_OFF,0,0};
static Voice noi = {48,0,0,0,0,0xFF,ENV_OFF,0,0};
static unsigned char tri_note = 48;
static unsigned char tri_gate = 0;
static unsigned char tri_trig = 0;
static unsigned char tri_last_note = 0;
static unsigned char tri_last_gate = 0;
static unsigned char tri_last_trig = 0xFF;
static unsigned char duty = 2;
static unsigned char env_a = 0;
static unsigned char env_d = 0;
static unsigned char env_s = 15;
static unsigned char env_r = 0;
static unsigned int p1_base_timer = 0;
static unsigned int p2_base_timer = 0;
static unsigned char p1_written_lo = 0xFF;
static unsigned char p2_written_lo = 0xFF;
static unsigned char p1_written_hi = 0;
static unsigned char p2_written_hi = 0;
static unsigned char p1_vibrato_phase = 0;
static unsigned char p1_vibrato_div = 0;
static signed char p1_vibrato_offset = 0;
static unsigned char lfo_depth = 0;
static unsigned char lfo_rate = 0;

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

static unsigned char bus_encode(unsigned char opcode) {
  return (unsigned char)(((opcode & 0x01) << 2) | (opcode & 0x02) | ((opcode & 0x04) >> 2));
}

static unsigned char read_phase(unsigned char opcode) {
  JOY_STROBE = bus_encode(opcode);
  delay_short(6);
  return (unsigned char)(JOY2_PORT & 0x1F);
}

static void write_idle(void) {
  JOY_STROBE = bus_encode(V2_IDLE);
  delay_short(2);
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

static void restart_env(Voice* v) {
  v->phase = ENV_ATTACK;
  v->level = 0;
  v->counter = 0;
}

static void release_env(Voice* v) {
  if (v->phase != ENV_OFF) {
    v->phase = ENV_RELEASE;
    v->counter = 0;
  }
}

static void tick_env(Voice* v) {
  unsigned char rate;
  switch (v->phase) {
    case ENV_ATTACK:
      rate = (unsigned char)(1 + env_a);
      if (++v->counter >= rate) {
        v->counter = 0;
        if (v->level < 15) ++v->level;
        if (v->level >= 15) v->phase = ENV_DECAY;
      }
      break;
    case ENV_DECAY:
      rate = (unsigned char)(1 + env_d);
      if (++v->counter >= rate) {
        v->counter = 0;
        if (v->level > env_s) --v->level;
        if (v->level <= env_s) v->phase = ENV_SUSTAIN;
      }
      break;
    case ENV_RELEASE:
      rate = (unsigned char)(1 + env_r);
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

static void write_pulse1_timer(unsigned char note) {
  unsigned int timer = timer_from_note(note);
  p1_base_timer = timer;
  p1_written_lo = (unsigned char)(timer & 0xFF);
  p1_written_hi = (unsigned char)((timer >> 8) & 0x07);
  P1_SWEEP = 0x08;
  P1_LO = p1_written_lo;
  P1_HI = p1_written_hi;
}

static void write_pulse2_timer(unsigned char note) {
  unsigned int timer = timer_from_note(note);
  p2_base_timer = timer;
  p2_written_lo = (unsigned char)(timer & 0xFF);
  p2_written_hi = (unsigned char)((timer >> 8) & 0x07);
  P2_SWEEP = 0x08;
  P2_LO = p2_written_lo;
  P2_HI = p2_written_hi;
}

static unsigned char ctrl_to_vibrato_depth(unsigned char v) {
  if (v < 16) return 0;
  return (unsigned char)(1 + (((unsigned int)(v - 16) * 18U) / 111U));
}

static unsigned char ctrl_to_vibrato_phase_step(unsigned char v) {
  if (v < 8) return 0;
  if (v < 18) return 2;
  if (v < 30) return 3;
  if (v < 44) return 4;
  if (v < 58) return 5;
  if (v < 72) return 6;
  if (v < 86) return 10;
  if (v < 100) return 22;
  if (v < 112) return 40;
  if (v < 120) return 72;
  return 120;
}

static signed char vibrato_step(unsigned char depth_ctrl, unsigned char rate_ctrl, unsigned char* phase) {
  unsigned char depth;
  unsigned char phase_step;
  signed char shape;
  int scaled;

  depth = ctrl_to_vibrato_depth(depth_ctrl);
  phase_step = ctrl_to_vibrato_phase_step(rate_ctrl);
  if (depth == 0 || phase_step == 0) {
    *phase = 0;
    return 0;
  }

  *phase = (unsigned char)(*phase + phase_step);
  shape = vibrato_shape[(*phase >> 3) & 0x1F];
  scaled = ((int)shape * (int)depth);
  if (scaled >= 0) scaled += 8;
  else scaled -= 8;
  return (signed char)(scaled / 16);
}

static void write_pulse_vibrato(Voice* v, unsigned char which, unsigned int base, signed char offset) {
  int mod_timer;
  unsigned char hi;
  unsigned char lo;

  if (!v->gate || base == 0) return;

  mod_timer = (int)base + (int)offset;
  if (mod_timer < 8) mod_timer = 8;
  if (mod_timer > 2047) mod_timer = 2047;

  lo = (unsigned char)(mod_timer & 0xFF);
  hi = (unsigned char)((mod_timer >> 8) & 0x07);
  if (which == 1) {
    if (lo != p1_written_lo) {
      p1_written_lo = lo;
      P1_LO = lo;
    }
    if (hi != p1_written_hi) {
      p1_written_hi = hi;
      P1_HI = hi;
    }
  } else {
    if (lo != p2_written_lo) {
      p2_written_lo = lo;
      P2_LO = lo;
    }
    if (hi != p2_written_hi) {
      p2_written_hi = hi;
      P2_HI = hi;
    }
  }
}

static void update_musical_vibrato(void) {
  ++p1_vibrato_div;
  if (p1_vibrato_div < 2) {
    return;
  }
  p1_vibrato_div = 0;

  p1_vibrato_offset = vibrato_step(112, 86, &p1_vibrato_phase);
  write_pulse_vibrato(&p1, 1, p1_base_timer, p1_vibrato_offset);
}

static void update_pulse(Voice* v, unsigned char which) {
  unsigned char vol;
  unsigned char duty_bits;

  if (v->gate && (!v->last_gate || v->note != v->last_note || v->trig != v->last_trig)) {
    restart_env(v);
    if (which == 1) {
      P1_VOL = 0x30;
      write_pulse1_timer(v->note);
    } else {
      P2_VOL = 0x30;
      write_pulse2_timer(v->note);
    }
  } else if (!v->gate && v->last_gate) {
    release_env(v);
  }

  tick_env(v);
  duty_bits = (unsigned char)((duty & 0x03) << 6);
  vol = (unsigned char)(duty_bits | 0x20 | 0x10 | (v->level & 0x0F));
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
      timer = timer_from_note(tri_note) >> 1;
      TRI_LINEAR = 0x80 | 0x7F;
      TRI_LO = (unsigned char)(timer & 0xFF);
      TRI_HI = (unsigned char)((timer >> 8) & 0x07);
    }
  } else if (tri_last_gate) {
    TRI_LINEAR = 0;
  }
  tri_last_note = tri_note;
  tri_last_gate = tri_gate;
  tri_last_trig = tri_trig;
}

static const DmcSample* dmc_sample_from_note(unsigned char note) {
  if (note >= 36 && note <= 37) return &dmc_kick;
  if (note >= 38 && note <= 40) return &dmc_snare;
  if (note == 41) return &dmc_rim;
  if (note == 46) return &dmc_voice;
  return 0;
}

static void trigger_dmc(const DmcSample* sample) {
  APU_CTRL = 0x0F;
  DMC_FREQ = (unsigned char)(sample->rate & 0x0F);
  DMC_RAW = (unsigned char)(sample->level & 0x7F);
  DMC_START = sample->start;
  DMC_LEN = sample->length;
  APU_CTRL = 0x1F;
}

static void update_noise_or_dmc(void) {
  const DmcSample* sample;
  unsigned char changed;
  changed = (unsigned char)(noi.gate && (!noi.last_gate || noi.note != noi.last_note || noi.trig != noi.last_trig));
  if (changed) {
    sample = dmc_sample_from_note(noi.note);
    if (sample != 0) {
      NOI_VOL = 0x30;
      trigger_dmc(sample);
    } else {
      restart_env(&noi);
      NOI_LO = noise_period_table[(noi.note >> 3) & 0x0F];
      NOI_HI = 0x18;
      noi.level = 15;
      noi.phase = ENV_SUSTAIN;
    }
  } else if (!noi.gate && noi.last_gate) {
    release_env(&noi);
  }

  tick_env(&noi);
  if (dmc_sample_from_note(noi.note) == 0) {
    NOI_VOL = (unsigned char)(0x20 | 0x10 | (noi.level & 0x0F));
  }

  noi.last_note = noi.note;
  noi.last_gate = noi.gate;
  noi.last_trig = noi.trig;
}

static void apply_reg(unsigned char reg_id, unsigned char value) {
  switch (reg_id & 0x0F) {
    case REG_DUTY:
      duty = (unsigned char)(value & 0x03);
      lfo_depth = (unsigned char)((value >> 2) & 0x07);
      lfo_rate = (unsigned char)((value >> 5) & 0x07);
      break;
    case REG_ADSR_A: env_a = ctrl_to_nibble(value); break;
    case REG_ADSR_D: env_d = ctrl_to_nibble(value); break;
    case REG_ADSR_S: env_s = ctrl_to_nibble(value); break;
    case REG_ADSR_R: env_r = ctrl_to_nibble(value); break;
    case REG_TRI_NOTE: tri_note = (unsigned char)(value & 0x7F); break;
    case REG_TRI_GATE: tri_gate = value ? 1 : 0; break;
    case REG_TRI_TRIG: tri_trig = value; break;
    case REG_P1_NOTE: p1.note = (unsigned char)(value & 0x7F); break;
    case REG_P1_GATE: p1.gate = value ? 1 : 0; break;
    case REG_P1_TRIG: p1.trig = value; break;
    case REG_P2_NOTE: p2.note = (unsigned char)(value & 0x7F); break;
    case REG_P2_GATE: p2.gate = value ? 1 : 0; break;
    case REG_P2_TRIG: p2.trig = value; break;
    case REG_NOI_NOTE: noi.note = (unsigned char)(value & 0x7F); ++noi.trig; break;
    case REG_NOI_GATE: noi.gate = value ? 1 : 0; break;
    default: break;
  }
}

static unsigned char service_one_event(void) {
  unsigned char status;
  unsigned char reg_field;
  unsigned char lo_field;
  unsigned char hi_field;
  unsigned char reg_id;
  unsigned char value;

  status = read_phase(V2_READ_STATUS);
  write_idle();
  if ((status & 0x11) != 0x11) return 0;
  reg_field = read_phase(V2_READ_REG_ID);
  write_idle();
  if ((reg_field & 0x10) == 0) return 0;
  lo_field = read_phase(V2_READ_VALUE_LO);
  write_idle();
  if ((lo_field & 0x10) == 0) return 0;
  hi_field = read_phase(V2_READ_VALUE_HI);
  write_idle();
  if ((hi_field & 0x10) == 0) return 0;

  reg_id = (unsigned char)(reg_field & 0x0F);
  value = (unsigned char)(((hi_field & 0x0F) << 4) | (lo_field & 0x0F));
  JOY_STROBE = bus_encode(V2_ACK_AND_NEXT);
  delay_short(6);
  write_idle();
  apply_reg(reg_id, value);
  return 1;
}

static void service_control_events(void) {
  unsigned char remaining = 12;
  while (remaining != 0) {
    if (!service_one_event()) return;
    --remaining;
  }
}

int main(void) {
  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x1F;
  DMC_RAW = 0x40;
  P1_VOL = 0x30;
  P2_VOL = 0x30;
  TRI_LINEAR = 0;
  NOI_VOL = 0x30;
  NOI_LO = 0x0F;

  unlock_transport();

  p1.note = 40;
  p1.gate = 1;
  p1.trig = 1;
  p2.gate = 0;
  tri_gate = 0;
  noi.gate = 0;
  duty = 2;
  env_a = 0;
  env_d = 0;
  env_s = 15;
  env_r = 0;

  for (;;) {
    update_pulse(&p1, 1);
    update_musical_vibrato();
    delay_short(4);
  }

  return 0;
}
