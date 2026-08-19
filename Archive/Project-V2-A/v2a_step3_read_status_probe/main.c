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

static void apply_sample(unsigned char sample) {
  switch (sample & 0x1F) {
    case 0x11:
      set_tone(0x50); // exact expected READ_STATUS
      break;
    case 0x01:
      set_tone(0x90); // D0 seen, VALID missing
      break;
    case 0x10:
      set_tone(0xD0); // VALID seen, D0 missing
      break;
    case 0x00:
      set_silence();  // no response
      break;
    default:
      set_tone(0x20); // wrong combination
      break;
  }
}

int main(void) {
  unsigned char last = 0xFF;
  unsigned char sample;
  unsigned char k;

  __asm__("sei");
  // Inhibit the APU frame IRQ so the tiny probe keeps running steadily.
  APU_FRAME = 0x40;
  APU_CTRL = 0x01;
  set_silence();

  // Unlock the Pico-side 245 safely.
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

  for (;;) {
    JOY_STROBE = 0x01; // READ_STATUS opcode
    delay_short(24);
    sample = (unsigned char)(JOY2_PORT & 0x1F);

    if (sample != last) {
      last = sample;
      apply_sample(sample);
    }

    delay_short(24);
  }

  return 0;
}
