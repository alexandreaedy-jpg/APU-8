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

static void set_tone(unsigned char lo) {
  PULSE1_SWEEP = 0x08;
  PULSE1_HI = 0x00;
  PULSE1_LO = lo;
  PULSE1_VOL = 0x3C;
}

static void set_silence(void) {
  PULSE1_VOL = 0x30;
}

static unsigned char read_phase(unsigned char opcode) {
  JOY_STROBE = opcode;
  delay_short(40);
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

static void step_phase(unsigned char opcode, unsigned char tone) {
  read_phase(opcode);
  set_tone(tone);
  delay_short(60);
}

int main(void) {
  __asm__("sei");
  APU_FRAME = 0x40;
  APU_CTRL = 0x01;
  set_silence();

  unlock_transport();

  for (;;) {
    // Event 0
    step_phase(0x01, 0xD0); // READ_STATUS
    step_phase(0x02, 0xB0); // READ_REG_ID
    step_phase(0x03, 0x90); // READ_VALUE_LO
    step_phase(0x04, 0x70); // READ_VALUE_HI
    step_phase(0x05, 0x50); // ACK_AND_NEXT

    // Event 1
    step_phase(0x01, 0xD0);
    step_phase(0x02, 0xB0);
    step_phase(0x03, 0x90);
    step_phase(0x04, 0x70);
    step_phase(0x05, 0x50);

    // Final empty status hold
    step_phase(0x01, 0x30);
  }

  return 0;
}
