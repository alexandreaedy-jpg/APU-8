#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define NOISE_VOL    (*(volatile unsigned char*)0x400C)
#define NOISE_LO     (*(volatile unsigned char*)0x400E)
#define NOISE_HI     (*(volatile unsigned char*)0x400F)
#define APU_FRAME    (*(volatile unsigned char*)0x4017)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

#define REG_NOI_NOTE 0x0E
#define REG_NOI_GATE 0x0F
#define REG_NOI_TRIG 0x04

#define V2_PROTO1_IDLE          0x00
#define V2_PROTO1_READ_STATUS   0x01
#define V2_PROTO1_READ_REG_ID   0x02
#define V2_PROTO1_READ_VALUE_LO 0x06
#define V2_PROTO1_READ_VALUE_HI 0x04
#define V2_PROTO1_ACK_AND_NEXT  0x05

static const unsigned char noise_period_table[16] = {
  0x0F, 0x0E, 0x0D, 0x0C,
  0x0B, 0x0A, 0x09, 0x08,
  0x07, 0x06, 0x05, 0x04,
  0x03, 0x02, 0x01, 0x00
};

static void delay_short(unsigned char outer) {
  volatile unsigned char i;
  volatile unsigned char j;

  for (i = 0; i < outer; ++i) {
    for (j = 0; j < 255; ++j) {
    }
  }
}

static unsigned char noise_period_from_note(unsigned char note) {
  return noise_period_table[(note >> 3) & 0x0F];
}

static unsigned char noise_level_from_note(unsigned char note) {
  if (note >= 72) {
    return 1;
  }
  if (note >= 60) {
    return 2;
  }
  if (note >= 48) {
    return 3;
  }
  return 5;
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
    case REG_NOI_NOTE:
      *note = (unsigned char)(value & 0x7F);
      break;
    case REG_NOI_GATE:
      *gate = value ? 1 : 0;
      break;
    case REG_NOI_TRIG:
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

static void update_noise(unsigned char note,
                         unsigned char gate,
                         unsigned char trig,
                         unsigned char* last_note,
                         unsigned char* last_gate,
                         unsigned char* last_trig) {
  unsigned char mode;
  unsigned char period;
  unsigned char level;

  if (gate) {
    if (note != *last_note || gate != *last_gate || trig != *last_trig) {
      mode = (note >= 84) ? 0x80 : 0x00;
      period = noise_period_from_note(note);
      level = noise_level_from_note(note);
      NOISE_VOL = level;
      NOISE_LO = mode | period;
      NOISE_HI = 0x18;
      *last_note = note;
      *last_trig = trig;
    }
  } else if (*last_gate != 0) {
    NOISE_VOL = 0x30;
  }

  *last_gate = gate;
}

int main(void) {
  unsigned char note = 48;
  unsigned char gate = 0;
  unsigned char trig = 0;
  unsigned char last_note = 0;
  unsigned char last_gate = 0;
  unsigned char last_trig = 0xFF;

  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x08;
  NOISE_VOL = 0x30;
  NOISE_LO = 0x0F;

  unlock_transport();

  for (;;) {
    service_control_events(&note, &gate, &trig);
    update_noise(note, gate, trig, &last_note, &last_gate, &last_trig);
    delay_short(2);
  }

  return 0;
}
