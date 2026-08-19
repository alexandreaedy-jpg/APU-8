#define PPU_STATUS   (*(volatile unsigned char*)0x2002)
#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)

#define JOY_STROBE   (*(volatile unsigned char*)0x4016)

static void delay_short(void) {
  volatile unsigned int i;
  for (i = 0; i < 5000U; ++i) {
  }
}

static void delay_hold(void) {
  unsigned char i;
  for (i = 0; i < 80; ++i) {
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
  PULSE1_LO = 0x60;
  PULSE1_HI = 0x00;
}

static void set_heartbeat_level(unsigned char level) {
  PULSE1_VOL = (unsigned char)(0x30 | (level & 0x0F));
}

static void set_note(unsigned char timerLo, unsigned char volume) {
  PULSE1_LO = timerLo;
  set_heartbeat_level(volume);
}

static void silence_note(void) {
  set_heartbeat_level(0);
}

static void play_boot_signature(void) {
  set_note(0x40, 12);
  delay_hold();
  silence_note();
  delay_short();

  set_note(0x60, 12);
  delay_hold();
  silence_note();
  delay_short();

  set_note(0x80, 12);
  delay_hold();
  silence_note();
  delay_hold();
}

int main(void) {
  static const unsigned char patterns[4] = { 0x01, 0x02, 0x04, 0x00 };
  static const unsigned char noteTimers[4] = { 0x30, 0x50, 0x70, 0xA0 };
  static const unsigned char noteVolumes[4] = { 12, 10, 8, 4 };
  unsigned char i;

  setup_apu_heartbeat();
  JOY_STROBE = 0;
  play_boot_signature();

  for (;;) {
    for (i = 0; i < 4; ++i) {
      JOY_STROBE = patterns[i];
      set_note(noteTimers[i], noteVolumes[i]);
      delay_hold();
      wait_vblank_rough();
    }
  }

  return 0;
}
