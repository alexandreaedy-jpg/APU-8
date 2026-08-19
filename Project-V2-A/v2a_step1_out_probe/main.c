#define PPU_STATUS   (*(volatile unsigned char*)0x2002)
#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)

#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

static void delay_short(void) {
  volatile unsigned int i;
  for (i = 0; i < 4000U; ++i) {
  }
}

static void delay_long(void) {
  unsigned char i;
  for (i = 0; i < 24; ++i) {
    delay_short();
  }
}

static void wait_vblank_rough(void) {
  while ((PPU_STATUS & 0x80) != 0) {
  }
  while ((PPU_STATUS & 0x80) == 0) {
  }
}

static void setup_apu_heartbeat(void) {
  APU_CTRL = 0x0F;
  PULSE1_SWEEP = 0x08;
  PULSE1_LO = 0x7F;
  PULSE1_HI = 0x00;
}

static void set_heartbeat_level(unsigned char level) {
  // duty 50%, constant volume envelope
  PULSE1_VOL = (unsigned char)(0x30 | (level & 0x0F));
}

int main(void) {
  unsigned char cmd;
  unsigned char readback;

  setup_apu_heartbeat();
  JOY_STROBE = 0;

  for (;;) {
    for (cmd = 0; cmd < 8; ++cmd) {
      JOY_STROBE = cmd;
      set_heartbeat_level((unsigned char)(8 + cmd));
      delay_long();

      // Generate two explicit $4017 reads so /OE2 visibly pulses.
      readback = JOY2_PORT;
      (void)readback;
      delay_short();
      readback = JOY2_PORT;
      (void)readback;

      wait_vblank_rough();
    }
  }

  return 0;
}
