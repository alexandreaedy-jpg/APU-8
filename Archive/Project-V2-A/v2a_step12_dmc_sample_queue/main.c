#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define DMC_FREQ     (*(volatile unsigned char*)0x4010)
#define DMC_RAW      (*(volatile unsigned char*)0x4011)
#define DMC_START    (*(volatile unsigned char*)0x4012)
#define DMC_LEN      (*(volatile unsigned char*)0x4013)
#define APU_FRAME    (*(volatile unsigned char*)0x4017)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

#define APU_ENABLE_STD 0x0F
#define APU_ENABLE_DMC 0x1F

#define REG_DMC_NOTE 0x0E
#define REG_DMC_GATE 0x0F
#define REG_DMC_TRIG 0x04

#define V2_PROTO1_IDLE          0x00
#define V2_PROTO1_READ_STATUS   0x01
#define V2_PROTO1_READ_REG_ID   0x02
#define V2_PROTO1_READ_VALUE_LO 0x06
#define V2_PROTO1_READ_VALUE_HI 0x04
#define V2_PROTO1_ACK_AND_NEXT  0x05

typedef struct DmcSample {
  unsigned char rate;
  unsigned char start;
  unsigned char length;
  unsigned char level;
} DmcSample;

static const DmcSample dmc_kick = {0x0C, 0xC0, 0x34, 0x44};
static const DmcSample dmc_snare = {0x0C, 0xCE, 0x1C, 0x42};
static const DmcSample dmc_rim = {0x0C, 0xD6, 0x0F, 0x3A};
static const DmcSample dmc_voice = {0x0C, 0xDA, 0x94, 0x34};

static void delay_short(unsigned char outer) {
  volatile unsigned char i;
  volatile unsigned char j;

  for (i = 0; i < outer; ++i) {
    for (j = 0; j < 255; ++j) {
    }
  }
}

static unsigned char v2_proto1_bus_encode(unsigned char opcode) {
  return (unsigned char)(((opcode & 0x01) << 2) |
                         (opcode & 0x02) |
                         ((opcode & 0x04) >> 2));
}

static unsigned char read_phase(unsigned char opcode) {
  JOY_STROBE = v2_proto1_bus_encode(opcode);
  delay_short(20);
  return (unsigned char)(JOY2_PORT & 0x1F);
}

static void write_idle(void) {
  JOY_STROBE = v2_proto1_bus_encode(V2_PROTO1_IDLE);
  delay_short(8);
}

static void unlock_transport(void) {
  unsigned char k;

  for (k = 0; k != 4; ++k) {
    JOY_STROBE = v2_proto1_bus_encode(0x04);
    delay_short(180);
    JOY_STROBE = v2_proto1_bus_encode(0x02);
    delay_short(180);
    JOY_STROBE = v2_proto1_bus_encode(0x01);
    delay_short(180);
    JOY_STROBE = v2_proto1_bus_encode(0x07);
    delay_short(180);
  }

  write_idle();
}

static const DmcSample* dmc_sample_from_note(unsigned char note) {
  if (note >= 36 && note <= 37) {
    return &dmc_kick;
  }
  if (note >= 38 && note <= 40) {
    return &dmc_snare;
  }
  if (note == 41) {
    return &dmc_rim;
  }
  if (note == 46) {
    return &dmc_voice;
  }
  return 0;
}

static void dmc_trigger(const DmcSample* sample) {
  APU_CTRL = APU_ENABLE_STD;
  DMC_FREQ = (unsigned char)(sample->rate & 0x0F);
  DMC_RAW = (unsigned char)(sample->level & 0x7F);
  DMC_START = sample->start;
  DMC_LEN = sample->length;
  APU_CTRL = APU_ENABLE_DMC;
}

static unsigned char service_one_event(unsigned char* note,
                                       unsigned char* gate,
                                       unsigned char* trig) {
  unsigned char status;
  unsigned char reg_field;
  unsigned char lo_field;
  unsigned char hi_field;
  unsigned char reg_id;
  unsigned char value;

  status = read_phase(V2_PROTO1_READ_STATUS);
  write_idle();
  if ((status & 0x11) != 0x11) {
    return 0;
  }

  reg_field = read_phase(V2_PROTO1_READ_REG_ID);
  write_idle();
  if ((reg_field & 0x10) == 0) {
    return 0;
  }

  lo_field = read_phase(V2_PROTO1_READ_VALUE_LO);
  write_idle();
  if ((lo_field & 0x10) == 0) {
    return 0;
  }

  hi_field = read_phase(V2_PROTO1_READ_VALUE_HI);
  write_idle();
  if ((hi_field & 0x10) == 0) {
    return 0;
  }

  reg_id = (unsigned char)(reg_field & 0x0F);
  value = (unsigned char)(((hi_field & 0x0F) << 4) | (lo_field & 0x0F));

  JOY_STROBE = v2_proto1_bus_encode(V2_PROTO1_ACK_AND_NEXT);
  delay_short(20);
  write_idle();

  switch (reg_id) {
    case REG_DMC_NOTE:
      *note = (unsigned char)(value & 0x7F);
      break;
    case REG_DMC_GATE:
      *gate = value ? 1 : 0;
      break;
    case REG_DMC_TRIG:
      *trig = value;
      break;
    default:
      break;
  }

  return 1;
}

static void service_control_events(unsigned char* note,
                                   unsigned char* gate,
                                   unsigned char* trig) {
  unsigned char remaining = 6;

  while (remaining != 0) {
    if (!service_one_event(note, gate, trig)) {
      return;
    }
    --remaining;
  }
}

static void update_dmc(unsigned char note,
                       unsigned char gate,
                       unsigned char trig,
                       unsigned char* last_note,
                       unsigned char* last_gate,
                       unsigned char* last_trig) {
  const DmcSample* sample;

  if (gate) {
    if (note != *last_note || gate != *last_gate || trig != *last_trig) {
      sample = dmc_sample_from_note(note);
      if (sample != 0) {
        dmc_trigger(sample);
      }
      *last_note = note;
      *last_trig = trig;
    }
  } else if (*last_gate != 0) {
    APU_CTRL = APU_ENABLE_STD;
  }

  *last_gate = gate;
}

int main(void) {
  unsigned char note = 36;
  unsigned char gate = 0;
  unsigned char trig = 0;
  unsigned char last_note = 0;
  unsigned char last_gate = 0;
  unsigned char last_trig = 0xFF;

  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = APU_ENABLE_STD;
  DMC_RAW = 0x40;

  unlock_transport();

  for (;;) {
    service_control_events(&note, &gate, &trig);
    update_dmc(note, gate, trig, &last_note, &last_gate, &last_trig);
    delay_short(2);
  }

  return 0;
}
