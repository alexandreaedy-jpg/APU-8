#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)
#define APU_FRAME    (*(volatile unsigned char*)0x4017)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

#define REG_DUTY        0x00
#define REG_ADSR_A      0x01
#define REG_ADSR_D      0x02
#define REG_ADSR_S      0x03
#define REG_ADSR_R      0x04

#define FOCUS_ATTACK    0
#define FOCUS_DECAY     1
#define FOCUS_SUSTAIN   2
#define FOCUS_RELEASE   3

static void delay_short(unsigned char outer) {
  volatile unsigned char i;
  volatile unsigned char j;

  for (i = 0; i < outer; ++i) {
    for (j = 0; j < 255; ++j) {
    }
  }
}

static void set_level(unsigned char duty, unsigned char level) {
  PULSE1_VOL = (unsigned char)(((duty & 0x03) << 6) | 0x10 | (level & 0x0F));
}

static void set_base_tone(unsigned char duty) {
  PULSE1_SWEEP = 0x08;
  PULSE1_LO = 0x70;
  PULSE1_HI = 0x00;
  set_level(duty, 0);
}

static unsigned char read_phase(unsigned char opcode) {
  JOY_STROBE = opcode;
  delay_short(20);
  return (unsigned char)(JOY2_PORT & 0x1F);
}

static void unlock_transport(void) {
  unsigned char k;

  for (k = 0; k != 4; ++k) {
    JOY_STROBE = 0x01;
    delay_short(180);
    JOY_STROBE = 0x02;
    delay_short(180);
    JOY_STROBE = 0x04;
    delay_short(180);
    JOY_STROBE = 0x07;
    delay_short(180);
  }
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
  return (unsigned char)((sustain * 15) / 15);
}

static void play_note_with_adsr(unsigned char duty,
                                unsigned char attack,
                                unsigned char decay,
                                unsigned char sustain,
                                unsigned char release,
                                unsigned char focus) {
  unsigned char level;
  unsigned char sustain_level;
  unsigned char attack_step_delay;
  unsigned char decay_step_delay;
  unsigned char release_step_delay;
  unsigned char sustain_hold;
  unsigned char post_silence;

  attack_step_delay = attack_delay_outer(attack);
  decay_step_delay = decay_delay_outer(decay);
  release_step_delay = release_delay_outer(release);
  sustain_level = sustain_level_from_value(sustain & 0x0F);

  switch (focus) {
    case FOCUS_ATTACK:
      sustain_hold = 10;
      post_silence = 12;
      break;
    case FOCUS_DECAY:
      sustain_hold = 18;
      post_silence = 12;
      break;
    case FOCUS_SUSTAIN:
      sustain_hold = 50;
      post_silence = 12;
      break;
    case FOCUS_RELEASE:
    default:
      sustain_hold = 14;
      post_silence = 22;
      break;
  }

  PULSE1_HI = 0x00;
  set_level(duty, 0);

  for (level = 0; level != 16; ++level) {
    set_level(duty, level);
    delay_short(attack_step_delay);
  }

  while (level > sustain_level) {
    --level;
    set_level(duty, level);
    delay_short(decay_step_delay);
  }

  delay_short(sustain_hold);

  while (level > 0) {
    --level;
    set_level(duty, level);
    delay_short(release_step_delay);
  }

  set_level(duty, 0);
  delay_short(post_silence);
}

static unsigned char service_control_events(unsigned char* duty_value,
                                            unsigned char* attack_value,
                                            unsigned char* decay_value,
                                            unsigned char* sustain_value,
                                            unsigned char* release_value,
                                            unsigned char* focus) {
  unsigned char status;
  unsigned char reg_field;
  unsigned char lo_field;
  unsigned char hi_field;
  unsigned char reg_id;
  unsigned char value;
  unsigned char consumed;

  consumed = 0;
  status = read_phase(0x01);
  if ((status & 0x11) != 0x11) {
    return 0;
  }

  reg_field = read_phase(0x02);
  lo_field = read_phase(0x03);
  hi_field = read_phase(0x04);

  reg_id = (unsigned char)(reg_field & 0x0F);
  value = (unsigned char)(((hi_field & 0x0F) << 4) | (lo_field & 0x0F));

  JOY_STROBE = 0x05;
  delay_short(20);

  value &= 0x0F;
  switch (reg_id) {
    case REG_DUTY:
      *duty_value = (unsigned char)(value & 0x03);
      break;
    case REG_ADSR_A:
      *attack_value = value;
      *focus = FOCUS_ATTACK;
      break;
    case REG_ADSR_D:
      *decay_value = value;
      *focus = FOCUS_DECAY;
      break;
    case REG_ADSR_S:
      *sustain_value = value;
      *focus = FOCUS_SUSTAIN;
      break;
    case REG_ADSR_R:
      *release_value = value;
      *focus = FOCUS_RELEASE;
      break;
    default:
      break;
  }

  consumed = 1;
  return consumed;
}

int main(void) {
  unsigned char duty_value = 0x02;
  unsigned char attack_value = 0x01;
  unsigned char decay_value = 0x04;
  unsigned char sustain_value = 0x08;
  unsigned char release_value = 0x03;
  unsigned char focus = FOCUS_ATTACK;

  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x01;
  set_base_tone(duty_value);

  unlock_transport();

  for (;;) {
    service_control_events(&duty_value,
                           &attack_value,
                           &decay_value,
                           &sustain_value,
                           &release_value,
                           &focus);
    play_note_with_adsr(duty_value,
                        attack_value,
                        decay_value,
                        sustain_value,
                        release_value,
                        focus);
  }

  return 0;
}
