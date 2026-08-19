#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)
#define PULSE2_VOL   (*(volatile unsigned char*)0x4004)
#define PULSE2_SWEEP (*(volatile unsigned char*)0x4005)
#define PULSE2_LO    (*(volatile unsigned char*)0x4006)
#define PULSE2_HI    (*(volatile unsigned char*)0x4007)
#define APU_FRAME    (*(volatile unsigned char*)0x4017)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

#define REG_P1_NOTE  0x08
#define REG_P1_GATE  0x09
#define REG_P1_TRIG  0x0A
#define REG_P2_NOTE  0x0B
#define REG_P2_GATE  0x0C
#define REG_P2_TRIG  0x0D

#define V2_PROTO1_IDLE          0x00
#define V2_PROTO1_READ_STATUS   0x01
#define V2_PROTO1_READ_REG_ID   0x02
#define V2_PROTO1_READ_VALUE_LO 0x06
#define V2_PROTO1_READ_VALUE_HI 0x04
#define V2_PROTO1_ACK_AND_NEXT  0x05

static const unsigned int pulse_timer_table[] = {
  1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906,
  856,808,762,720,678,640,604,570,538,508,480,453,
  428,404,381,360,339,320,302,285,269,254,240,226,
  214,202,190,180,170,160,151,143,135,127,120,113,
  107,101,95,90,85,80,75,71,67,63,60,56,
  53,50,47,45,42,40,37,35,33,31,30,28
};

static void delay_short(unsigned char outer) {
  volatile unsigned char i;
  volatile unsigned char j;

  for (i = 0; i < outer; ++i) {
    for (j = 0; j < 255; ++j) {
    }
  }
}

static unsigned int pulse_timer_from_note(unsigned char note) {
  if (note < 24) {
    note = 24;
  }
  if (note > 95) {
    note = 95;
  }
  return pulse_timer_table[note - 24];
}

static void set_pulse1_level(unsigned char level) {
  PULSE1_VOL = (unsigned char)(0x80 | 0x10 | (level & 0x0F));
}

static void set_pulse2_level(unsigned char level) {
  PULSE2_VOL = (unsigned char)(0x40 | 0x10 | (level & 0x0F));
}

static void set_pulse1_note(unsigned char note) {
  unsigned int timer = pulse_timer_from_note(note);
  PULSE1_SWEEP = 0x08;
  PULSE1_LO = (unsigned char)(timer & 0xFF);
  PULSE1_HI = (unsigned char)((timer >> 8) & 0x07);
}

static void set_pulse2_note(unsigned char note) {
  unsigned int timer = pulse_timer_from_note(note);
  PULSE2_SWEEP = 0x08;
  PULSE2_LO = (unsigned char)(timer & 0xFF);
  PULSE2_HI = (unsigned char)((timer >> 8) & 0x07);
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

static unsigned char service_one_event(unsigned char* p1_note,
                                       unsigned char* p1_gate,
                                       unsigned char* p1_trig,
                                       unsigned char* p2_note,
                                       unsigned char* p2_gate,
                                       unsigned char* p2_trig) {
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
    case REG_P1_NOTE:
      *p1_note = (unsigned char)(value & 0x7F);
      break;
    case REG_P1_GATE:
      *p1_gate = value ? 1 : 0;
      break;
    case REG_P1_TRIG:
      *p1_trig = value;
      break;
    case REG_P2_NOTE:
      *p2_note = (unsigned char)(value & 0x7F);
      break;
    case REG_P2_GATE:
      *p2_gate = value ? 1 : 0;
      break;
    case REG_P2_TRIG:
      *p2_trig = value;
      break;
    default:
      break;
  }

  return 1;
}

static void service_control_events(unsigned char* p1_note,
                                   unsigned char* p1_gate,
                                   unsigned char* p1_trig,
                                   unsigned char* p2_note,
                                   unsigned char* p2_gate,
                                   unsigned char* p2_trig) {
  unsigned char remaining = 8;

  while (remaining != 0) {
    if (!service_one_event(p1_note, p1_gate, p1_trig, p2_note, p2_gate, p2_trig)) {
      return;
    }
    --remaining;
  }
}

static void update_pulse1(unsigned char note,
                          unsigned char gate,
                          unsigned char trig,
                          unsigned char* last_note,
                          unsigned char* last_gate,
                          unsigned char* last_trig) {
  if (gate) {
    if (!*last_gate || note != *last_note || trig != *last_trig) {
      set_pulse1_level(0);
      set_pulse1_note(note);
      set_pulse1_level(10);
    }
  } else if (*last_gate) {
    set_pulse1_level(0);
  }

  *last_note = note;
  *last_gate = gate;
  *last_trig = trig;
}

static void update_pulse2(unsigned char note,
                          unsigned char gate,
                          unsigned char trig,
                          unsigned char* last_note,
                          unsigned char* last_gate,
                          unsigned char* last_trig) {
  if (gate) {
    if (!*last_gate || note != *last_note || trig != *last_trig) {
      set_pulse2_level(0);
      set_pulse2_note(note);
      set_pulse2_level(8);
    }
  } else if (*last_gate) {
    set_pulse2_level(0);
  }

  *last_note = note;
  *last_gate = gate;
  *last_trig = trig;
}

int main(void) {
  unsigned char p1_note = 60;
  unsigned char p1_gate = 0;
  unsigned char p1_trig = 0;
  unsigned char p2_note = 67;
  unsigned char p2_gate = 0;
  unsigned char p2_trig = 0;

  unsigned char last_p1_note = 0;
  unsigned char last_p1_gate = 0;
  unsigned char last_p1_trig = 0xFF;
  unsigned char last_p2_note = 0;
  unsigned char last_p2_gate = 0;
  unsigned char last_p2_trig = 0xFF;

  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x03;
  set_pulse1_level(0);
  set_pulse2_level(0);

  unlock_transport();

  for (;;) {
    service_control_events(&p1_note, &p1_gate, &p1_trig,
                           &p2_note, &p2_gate, &p2_trig);

    update_pulse1(p1_note, p1_gate, p1_trig,
                  &last_p1_note, &last_p1_gate, &last_p1_trig);
    update_pulse2(p2_note, p2_gate, p2_trig,
                  &last_p2_note, &last_p2_gate, &last_p2_trig);

    delay_short(2);
  }

  return 0;
}
