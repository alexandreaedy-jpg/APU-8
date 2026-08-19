#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)
#define JOY_STROBE   (*(volatile unsigned char*)0x4016)

static void delay_forever(void) {
  for (;;) {
  }
}

int main(void) {
  APU_CTRL = 0x0F;
  PULSE1_SWEEP = 0x08;
  PULSE1_LO = 0x60;
  PULSE1_HI = 0x00;
  PULSE1_VOL = 0x3C;

  JOY_STROBE = 0x02;

  delay_forever();
  return 0;
}
