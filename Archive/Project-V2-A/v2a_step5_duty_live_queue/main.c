#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)
#define APU_FRAME    (*(volatile unsigned char*)0x4017)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

static void delay_short(unsigned char outer) {
  volatile unsigned char i;
  volatile unsigned char j;

  for (i = 0; i < outer; ++i) {
    for (j = 0; j < 255; ++j) {
    }
  }
}

static void set_duty(unsigned char duty) {
  PULSE1_VOL = (unsigned char)(((duty & 0x03) << 6) | 0x3C);
}

static void retrigger_pulse(void) {
  PULSE1_HI = 0x00;
}

static void set_base_tone(void) {
  PULSE1_SWEEP = 0x08;
  PULSE1_HI = 0x00;
  PULSE1_LO = 0x88;
  set_duty(0);
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

int main(void) {
  unsigned char reg_field;
  unsigned char lo_field;
  unsigned char hi_field;
  unsigned char reg_id;
  unsigned char value;

  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x01;
  set_base_tone();

  unlock_transport();

  for (;;) {
    // Walk the whole packet every time. Step 4 already proved atomicity;
    // here we want an audible proof that the live queue can actually drain.
    read_phase(0x01);   /* READ_STATUS */
    reg_field = read_phase(0x02);
    lo_field = read_phase(0x03);
    hi_field = read_phase(0x04);

    reg_id = (unsigned char)(reg_field & 0x0F);
    value = (unsigned char)(((hi_field & 0x0F) << 4) | (lo_field & 0x0F));

    JOY_STROBE = 0x05;
    delay_short(20);

    if (reg_id == 0x00) {
      set_duty(value);
      retrigger_pulse();
    }

    // Let the timbre step breathe a little, but keep draining aggressively.
    delay_short(40);
  }

  return 0;
}
