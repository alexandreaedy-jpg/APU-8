#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)
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

static void set_silence(void) {
  PULSE1_VOL = 0x30;
}

static void set_tone(unsigned char lo) {
  PULSE1_SWEEP = 0x08;
  PULSE1_HI = 0x00;
  PULSE1_LO = lo;
  PULSE1_VOL = 0x3C;
}

static void apply_sample(unsigned char sample) {
  switch (sample & 0x1F) {
    case 0x01: set_tone(0xD0); break; // D0 only
    case 0x02: set_tone(0xB0); break; // D1 only
    case 0x04: set_tone(0x90); break; // D2 only
    case 0x08: set_tone(0x70); break; // D3 only
    case 0x10: set_tone(0x50); break; // D4 only
    case 0x00: set_silence(); break;  // idle gap
    default:   set_tone(0x20); break; // invalid / multiple bits high
  }
}

int main(void) {
  unsigned char last = 0xFF;
  unsigned char k;

  APU_CTRL = 0x01;
  set_silence();

  // Arm the Pico-side 74HCT245 deliberately with a repeated 4-step unlock
  // sequence. Repeating it makes the bring-up tolerant to timing mismatches.
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
    unsigned char sample = (unsigned char)(JOY2_PORT & 0x1F);

    if (sample != last) {
      last = sample;
      apply_sample(sample);
    }

    delay_short(24);
  }

  return 0;
}
