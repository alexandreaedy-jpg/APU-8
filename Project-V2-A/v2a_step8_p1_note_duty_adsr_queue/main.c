#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)
#define APU_FRAME    (*(volatile unsigned char*)0x4017)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

#define REG_DUTY     0x00
#define REG_ADSR_A   0x01
#define REG_ADSR_D   0x02
#define REG_ADSR_S   0x03
#define REG_ADSR_R   0x04
#define REG_P1_NOTE  0x08
#define REG_P1_GATE  0x09
#define REG_P1_TRIG  0x0A

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

static void set_level(unsigned char duty, unsigned char level) {
  PULSE1_VOL = (unsigned char)(((duty & 0x03) << 6) | 0x10 | (level & 0x0F));
}

static void set_note(unsigned char note, unsigned char duty) {
  unsigned int timer = pulse_timer_from_note(note);
  PULSE1_SWEEP = 0x08;
  PULSE1_LO = (unsigned char)(timer & 0xFF);
  PULSE1_HI = (unsigned char)((timer >> 8) & 0x07);
  set_level(duty, 0);
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

static unsigned char attack_delay_outer(unsigned char attack) {
  if (attack < 0x02) {
    return 1;
  }
  if (attack < 0x05) {
    return 3;
  }
  if (attack < 0x0A) {
    return 6;
  }
  return 10;
}

static unsigned char decay_delay_outer(unsigned char decay) {
  if (decay < 0x02) {
    return 1;
  }
  if (decay < 0x05) {
    return 2;
  }
  if (decay < 0x09) {
    return 4;
  }
  if (decay < 0x0D) {
    return 6;
  }
  return 9;
}

static unsigned char release_delay_outer(unsigned char release) {
  if (release < 0x02) {
    return 1;
  }
  if (release < 0x05) {
    return 2;
  }
  if (release < 0x09) {
    return 4;
  }
  if (release < 0x0D) {
    return 7;
  }
  return 10;
}

static unsigned char sustain_level_from_value(unsigned char sustain) {
  return (unsigned char)(sustain & 0x0F);
}

static void play_note_with_adsr(unsigned char note,
                                unsigned char duty,
                                unsigned char attack,
                                unsigned char decay,
                                unsigned char sustain,
                                unsigned char release) {
  unsigned char level;
  unsigned char sustain_level;
  unsigned char attack_step_delay;
  unsigned char decay_step_delay;
  unsigned char release_step_delay;

  attack_step_delay = attack_delay_outer(attack);
  decay_step_delay = decay_delay_outer(decay);
  release_step_delay = release_delay_outer(release);
  sustain_level = sustain_level_from_value(sustain);

  set_note(note, duty);

  for (level = 0; level != 16; ++level) {
    set_level(duty, level);
    delay_short(attack_step_delay);
  }

  while (level > sustain_level) {
    --level;
    set_level(duty, level);
    delay_short(decay_step_delay);
  }

  delay_short(18);

  while (level > 0) {
    --level;
    set_level(duty, level);
    delay_short(release_step_delay);
  }

  set_level(duty, 0);
  delay_short(10);
}

static unsigned char service_one_event(unsigned char* duty_value,
                                       unsigned char* attack_value,
                                       unsigned char* decay_value,
                                       unsigned char* sustain_value,
                                       unsigned char* release_value,
                                       unsigned char* note_value,
                                       unsigned char* gate_value,
                                       unsigned char* trig_value) {
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
    case REG_DUTY:
      *duty_value = (unsigned char)(value & 0x03);
      break;
    case REG_ADSR_A:
      *attack_value = (unsigned char)(value & 0x0F);
      break;
    case REG_ADSR_D:
      *decay_value = (unsigned char)(value & 0x0F);
      break;
    case REG_ADSR_S:
      *sustain_value = (unsigned char)(value & 0x0F);
      break;
    case REG_ADSR_R:
      *release_value = (unsigned char)(value & 0x0F);
      break;
    case REG_P1_NOTE:
      *note_value = (unsigned char)(value & 0x7F);
      break;
    case REG_P1_GATE:
      *gate_value = value ? 1 : 0;
      break;
    case REG_P1_TRIG:
      *trig_value = value;
      break;
    default:
      break;
  }

  return 1;
}

static void service_control_events(unsigned char* duty_value,
                                   unsigned char* attack_value,
                                   unsigned char* decay_value,
                                   unsigned char* sustain_value,
                                   unsigned char* release_value,
                                   unsigned char* note_value,
                                   unsigned char* gate_value,
                                   unsigned char* trig_value) {
  unsigned char remaining = 6;

  while (remaining != 0) {
    if (!service_one_event(duty_value,
                           attack_value,
                           decay_value,
                           sustain_value,
                           release_value,
                           note_value,
                           gate_value,
                           trig_value)) {
      return;
    }
    --remaining;
  }
}

int main(void) {
  unsigned char duty_value = 0x02;
  unsigned char attack_value = 0x04;
  unsigned char decay_value = 0x04;
  unsigned char sustain_value = 0x08;
  unsigned char release_value = 0x03;
  unsigned char note_value = 60;
  unsigned char gate_value = 1;
  unsigned char trig_value = 1;
  unsigned char last_note = 0;
  unsigned char last_gate = 0;
  unsigned char last_trig = 0xFF;

  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x01;
  set_level(duty_value, 0);

  unlock_transport();

  for (;;) {
    service_control_events(&duty_value,
                           &attack_value,
                           &decay_value,
                           &sustain_value,
                           &release_value,
                           &note_value,
                           &gate_value,
                           &trig_value);

    if (gate_value) {
      if (!last_gate || note_value != last_note || trig_value != last_trig) {
        play_note_with_adsr(note_value,
                            duty_value,
                            attack_value,
                            decay_value,
                            sustain_value,
                            release_value);
      } else {
        delay_short(2);
      }
    } else {
      set_level(duty_value, 0);
      delay_short(10);
    }

    last_note = note_value;
    last_gate = gate_value;
    last_trig = trig_value;
  }

  return 0;
}
