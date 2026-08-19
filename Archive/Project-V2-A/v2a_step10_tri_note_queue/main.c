#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define TRI_LINEAR   (*(volatile unsigned char*)0x4008)
#define TRI_LO       (*(volatile unsigned char*)0x400A)
#define TRI_HI       (*(volatile unsigned char*)0x400B)
#define APU_FRAME    (*(volatile unsigned char*)0x4017)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

#define REG_TRI_NOTE 0x05
#define REG_TRI_GATE 0x06
#define REG_TRI_TRIG 0x07

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

static unsigned int triangle_timer_from_note(unsigned char note) {
  return pulse_timer_from_note(note) >> 1;
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

static unsigned char service_one_event(unsigned char* tri_note,
                                       unsigned char* tri_gate,
                                       unsigned char* tri_trig) {
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
    case REG_TRI_NOTE:
      *tri_note = (unsigned char)(value & 0x7F);
      break;
    case REG_TRI_GATE:
      *tri_gate = value ? 1 : 0;
      break;
    case REG_TRI_TRIG:
      *tri_trig = value;
      break;
    default:
      break;
  }

  return 1;
}

static void service_control_events(unsigned char* tri_note,
                                   unsigned char* tri_gate,
                                   unsigned char* tri_trig) {
  unsigned char remaining = 6;

  while (remaining != 0) {
    if (!service_one_event(tri_note, tri_gate, tri_trig)) {
      return;
    }
    --remaining;
  }
}

static void update_triangle(unsigned char note,
                            unsigned char gate,
                            unsigned char trig,
                            unsigned char* last_note,
                            unsigned char* last_gate,
                            unsigned char* last_trig,
                            unsigned char* last_lo,
                            unsigned char* last_hi) {
  unsigned int timer;
  unsigned char lo;
  unsigned char hi;
  unsigned char new_note;

  if (gate) {
    timer = triangle_timer_from_note(note);
    lo = (unsigned char)(timer & 0xFF);
    hi = (unsigned char)((timer >> 8) & 0x07);
    new_note = (unsigned char)(note != *last_note || gate != *last_gate || trig != *last_trig);

    if (new_note) {
      TRI_LINEAR = 0x80 | 0x7F;
    }
    if (lo != *last_lo || new_note) {
      TRI_LO = lo;
      *last_lo = lo;
    }
    if (hi != *last_hi || new_note) {
      TRI_HI = hi;
      *last_hi = hi;
      *last_note = note;
      *last_trig = trig;
    }
  } else if (*last_gate != 0) {
    TRI_LINEAR = 0x00;
    *last_lo = 0xFF;
    *last_hi = 0xFF;
  }

  *last_gate = gate;
}

int main(void) {
  unsigned char tri_note = 60;
  unsigned char tri_gate = 0;
  unsigned char tri_trig = 0;
  unsigned char last_note = 0;
  unsigned char last_gate = 0;
  unsigned char last_trig = 0xFF;
  unsigned char last_lo = 0xFF;
  unsigned char last_hi = 0xFF;

  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x04;
  TRI_LINEAR = 0x00;

  unlock_transport();

  for (;;) {
    service_control_events(&tri_note, &tri_gate, &tri_trig);
    update_triangle(tri_note, tri_gate, tri_trig,
                    &last_note, &last_gate, &last_trig, &last_lo, &last_hi);
    delay_short(2);
  }

  return 0;
}
