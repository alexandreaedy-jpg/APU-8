#define PPU_CTRL     (*(volatile unsigned char*)0x2000)
#define PPU_MASK     (*(volatile unsigned char*)0x2001)
#define PPU_STATUS   (*(volatile unsigned char*)0x2002)
#define PPU_SCROLL   (*(volatile unsigned char*)0x2005)
#define PPU_ADDR     (*(volatile unsigned char*)0x2006)
#define PPU_DATA     (*(volatile unsigned char*)0x2007)
#define APU_CTRL     (*(volatile unsigned char*)0x4015)
#define DMC_FREQ     (*(volatile unsigned char*)0x4010)
#define DMC_RAW      (*(volatile unsigned char*)0x4011)
#define DMC_START    (*(volatile unsigned char*)0x4012)
#define DMC_LEN      (*(volatile unsigned char*)0x4013)

#define PULSE1_VOL   (*(volatile unsigned char*)0x4000)
#define PULSE1_SWEEP (*(volatile unsigned char*)0x4001)
#define PULSE1_LO    (*(volatile unsigned char*)0x4002)
#define PULSE1_HI    (*(volatile unsigned char*)0x4003)

#define PULSE2_VOL   (*(volatile unsigned char*)0x4004)
#define PULSE2_SWEEP (*(volatile unsigned char*)0x4005)
#define PULSE2_LO    (*(volatile unsigned char*)0x4006)
#define PULSE2_HI    (*(volatile unsigned char*)0x4007)

#define TRI_LINEAR   (*(volatile unsigned char*)0x4008)
#define TRI_LO       (*(volatile unsigned char*)0x400A)
#define TRI_HI       (*(volatile unsigned char*)0x400B)

#define NOISE_VOL    (*(volatile unsigned char*)0x400C)
#define NOISE_LO     (*(volatile unsigned char*)0x400E)
#define NOISE_HI     (*(volatile unsigned char*)0x400F)

#define JOY_STROBE   (*(volatile unsigned char*)0x4016)
#define JOY1_PORT    (*(volatile unsigned char*)0x4016)
#define JOY2_PORT    (*(volatile unsigned char*)0x4017)

#define BUS_SEQ          (*(volatile unsigned char*)0x0700)
#define BUS_ATTACK       (*(volatile unsigned char*)0x0701)
#define BUS_DECAY        (*(volatile unsigned char*)0x0702)
#define BUS_SUSTAIN      (*(volatile unsigned char*)0x0703)
#define BUS_RELEASE      (*(volatile unsigned char*)0x0704)
#define BUS_P1_LFO_TARGET (*(volatile unsigned char*)0x0705)
#define BUS_P2_LFO_TARGET (*(volatile unsigned char*)0x0706)
#define BUS_TRI_LFO_TARGET (*(volatile unsigned char*)0x0707)
#define BUS_MAGIC_A       (*(volatile unsigned char*)0x0708)
#define BUS_MAGIC_B       (*(volatile unsigned char*)0x0709)

#define BUS_P1_NOTE      (*(volatile unsigned char*)0x070A)
#define BUS_P1_VEL       (*(volatile unsigned char*)0x070B)
#define BUS_P1_GATE      (*(volatile unsigned char*)0x070C)
#define BUS_P2_NOTE      (*(volatile unsigned char*)0x070D)
#define BUS_P2_VEL       (*(volatile unsigned char*)0x070E)
#define BUS_P2_GATE      (*(volatile unsigned char*)0x070F)
#define BUS_TRI_NOTE     (*(volatile unsigned char*)0x0710)
#define BUS_TRI_VEL      (*(volatile unsigned char*)0x0711)
#define BUS_TRI_GATE     (*(volatile unsigned char*)0x0712)
#define BUS_NOI_NOTE     (*(volatile unsigned char*)0x0713)
#define BUS_NOI_VEL      (*(volatile unsigned char*)0x0714)
#define BUS_NOI_GATE     (*(volatile unsigned char*)0x0715)
#define BUS_P1_DUTY      (*(volatile unsigned char*)0x0716)
#define BUS_P1_GLIDE     (*(volatile unsigned char*)0x0717)
#define BUS_P1_VIB_DEPTH (*(volatile unsigned char*)0x0718)
#define BUS_P1_VIB_RATE  (*(volatile unsigned char*)0x0719)
#define BUS_P2_DUTY      (*(volatile unsigned char*)0x071A)
#define BUS_P2_GLIDE     (*(volatile unsigned char*)0x071B)
#define BUS_P2_VIB_DEPTH (*(volatile unsigned char*)0x071C)
#define BUS_P2_VIB_RATE  (*(volatile unsigned char*)0x071D)
#define BUS_TRI_GLIDE    (*(volatile unsigned char*)0x071E)
#define BUS_TRI_VIB_DEPTH (*(volatile unsigned char*)0x071F)
#define BUS_TRI_VIB_RATE  (*(volatile unsigned char*)0x0720)
#define BUS_P1_ATTACK    (*(volatile unsigned char*)0x0721)
#define BUS_P1_DECAY     (*(volatile unsigned char*)0x0722)
#define BUS_P1_SUSTAIN   (*(volatile unsigned char*)0x0723)
#define BUS_P1_RELEASE   (*(volatile unsigned char*)0x0724)
#define BUS_P2_ATTACK    (*(volatile unsigned char*)0x0725)
#define BUS_P2_DECAY     (*(volatile unsigned char*)0x0726)
#define BUS_P2_SUSTAIN   (*(volatile unsigned char*)0x0727)
#define BUS_P2_RELEASE   (*(volatile unsigned char*)0x0728)
#define BUS_TRI_ATTACK   (*(volatile unsigned char*)0x0729)
#define BUS_TRI_DECAY    (*(volatile unsigned char*)0x072A)
#define BUS_TRI_SUSTAIN  (*(volatile unsigned char*)0x072B)
#define BUS_TRI_RELEASE  (*(volatile unsigned char*)0x072C)
#define BUS_NOI_ATTACK   (*(volatile unsigned char*)0x072D)
#define BUS_NOI_DECAY    (*(volatile unsigned char*)0x072E)
#define BUS_NOI_SUSTAIN  (*(volatile unsigned char*)0x072F)
#define BUS_NOI_RELEASE  (*(volatile unsigned char*)0x0730)
#define BUS_NOI_TIMBRE   (*(volatile unsigned char*)0x0731)
#define BUS_NOI_MODE     (*(volatile unsigned char*)0x0732)
#define BUS_TRI_PUNCH    (*(volatile unsigned char*)0x0733)
#define BUS_P1_TRIG      (*(volatile unsigned char*)0x0734)
#define BUS_P2_TRIG      (*(volatile unsigned char*)0x0735)
#define BUS_TRI_TRIG     (*(volatile unsigned char*)0x0736)
#define BUS_NOI_TRIG     (*(volatile unsigned char*)0x0737)

#define BUS_BASE         ((volatile unsigned char*)0x0700)
#define BUS_PACKET_SIZE  58

// Hardware controller RX:
// - 0 = mode emulateur / fichier d'etat
// - 1 = lecture directe du Nano sur le port manette NES
#ifndef CONTROLLER_RX_ENABLED
#define CONTROLLER_RX_ENABLED 1
#endif

#ifndef CONTROLLER_RX_PORT
#define CONTROLLER_RX_PORT    2
#endif

#ifndef CONTROLLER_RX_INVERT
#define CONTROLLER_RX_INVERT  0
#endif

#ifndef CONTROLLER_RX_D3_INVERT
#define CONTROLLER_RX_D3_INVERT 0
#endif

#ifndef CONTROLLER_RX_MASK
#define CONTROLLER_RX_MASK    1    // DN0 = bit 0 du port $4017 (NES pin 2)
#endif

#ifndef CONTROLLER_RX_D3_MASK
#define CONTROLLER_RX_D3_MASK 8    // DN3 = bit 3 du port $4017 (NES pin 6, port 2 uniquement)
#endif

#ifndef CONTROLLER_RX_D4_MASK
#define CONTROLLER_RX_D4_MASK 16   // DN4 = bit 4 du port $4017 (NES pin 5, port 2 uniquement)
#endif

#ifndef CONTROLLER_RX_D4_INVERT
#define CONTROLLER_RX_D4_INVERT 0
#endif

#ifndef CONTROLLER_RX_POLL_DIV
#define CONTROLLER_RX_POLL_DIV 1
#endif

// Force mode simple pour éviter l'overflow mémoire (comme l'ancienne version fonctionnelle)
#undef CONTROLLER_RX_COMPACT
#define CONTROLLER_RX_COMPACT 0

#ifndef CONTROLLER_RX_P2_ONLY
#define CONTROLLER_RX_P2_ONLY 0
#endif

#ifndef CONTROLLER_RX_FAST_P2
#define CONTROLLER_RX_FAST_P2 0
#endif

#ifndef CONTROLLER_RX_FAST_IDLE_DELAY
#define CONTROLLER_RX_FAST_IDLE_DELAY 0
#endif

#ifndef BOOT_DIAG_ENABLED
#define BOOT_DIAG_ENABLED 0
#endif

// Suppression agressive des fonctionnalités pour libérer de la mémoire
#define DISABLE_ALL_BOOT_CODE 1
#define DISABLE_DIAG_SCREENS 1
#define MINIMAL_MEMORY_MODE 1

#define FAST_ENV_TICK_COUNT 80U
#define FAST_PULSE_MAX_LEVEL 10
#define SAFE_PULSE_SUSTAIN_LEVEL FAST_PULSE_MAX_LEVEL
#define SAFE_CTRL_MAX_STEP 8

#define CONTROLLER_RX_COMPACT_SIZE 12  // Format compact 12 bytes pour DN0 (P1+P2+TRI)

#ifndef DIAG_AUTOTONE
#define DIAG_AUTOTONE 0
#endif

// Optimisation mémoire : désactiver les fonctionnalités non essentielles
#define DISABLE_DIAG 1
#define DISABLE_BOOT_SCREEN 1
#define DISABLE_ALL_DIAG 1
#define MINIMAL_BUILD 1

#ifndef DIAG_PORT_RAW
#define DIAG_PORT_RAW 0
#endif

#ifndef DIAG_BIT_TONE
#define DIAG_BIT_TONE 0
#endif

#define APU_ENABLE_STD  0x0F
#define APU_ENABLE_DMC  0x1F

#define ENV_OFF     0
#define ENV_ATTACK  1
#define ENV_DECAY   2
#define ENV_SUSTAIN 3
#define ENV_RELEASE 4

#define DRUM_KICK        0
#define DRUM_TOM         1
#define DRUM_SNARE       2
#define DRUM_CLHAT       3
#define DRUM_OPHAT       4
#define DRUM_CYMBAL      5

#define LFO_TARGET_PITCH 0
#define LFO_TARGET_DUTY  1
#define LFO_TARGET_AMP   2

#define CONTROL_COMMAND_TRIGGER 0xFE
#define CONTROL_COMMAND_MAGIC   0x7D
#define CONTROL_PACKET_MAGIC    0x5A
#define CONTROL_TARGET_P1       0
#define CONTROL_TARGET_P2       1
#define CONTROL_TARGET_GLOBAL   3
#define CONTROL_PARAM_ATTACK    0
#define CONTROL_PARAM_DECAY     1
#define CONTROL_PARAM_SUSTAIN   2
#define CONTROL_PARAM_RELEASE   3

#define PAD_A      0x01
#define PAD_B      0x02
#define PAD_SELECT 0x04
#define PAD_START  0x08

const unsigned int pulse_timer_table[] = {
    1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906,
    856,808,762,720,678,640,604,570,538,508,480,453,
    428,404,381,360,339,320,302,285,269,254,240,226,
    214,202,190,180,170,160,151,143,135,127,120,113,
    107,101,95,90,85,80,75,71,67,63,60,56,
    53,50,47,45,42,40,37,35,33,31,30,28
};

const unsigned char noise_period_table[16] = {
    0x0F, 0x0E, 0x0D, 0x0C,
    0x0B, 0x0A, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04,
    0x03, 0x02, 0x01, 0x00
};

const signed char vibrato_shape[32] = {
     0,  3,  6,  9, 11, 13, 14, 15,
    16, 15, 14, 13, 11,  9,  6,  3,
     0, -3, -6, -9,-11,-13,-14,-15,
   -16,-15,-14,-13,-11, -9, -6, -3
};

typedef struct PulseVoice {
    unsigned char note;
    unsigned char velocity;
    unsigned char gate;
    unsigned char last_trigger;
    unsigned char duty;
    unsigned char env_phase;
    unsigned char env_counter;
    unsigned char env_volume;
    unsigned char last_lo;
    unsigned char last_hi;
    unsigned int current_t;
    unsigned int target_t;
} PulseVoice;

typedef struct TriangleVoice {
    unsigned char note;
    unsigned char gate;
    unsigned char last_trigger;
    unsigned char release_counter;
    unsigned char punch_counter;
    unsigned char punch_amount;
    unsigned char last_lo;
    unsigned char last_hi;
    unsigned int current_t;
    unsigned int target_t;
} TriangleVoice;

typedef struct NoiseVoice {
    unsigned char note;
    unsigned char velocity;
    unsigned char gate;
    unsigned char last_trigger;
    unsigned char drum_kind;
    unsigned char env_phase;
    unsigned char env_counter;
    unsigned char env_volume;
    unsigned char period;
    unsigned char last_period;
    unsigned char mode;
    unsigned char transient_frames;
    unsigned char transient_period;
    unsigned char transient_mode;
    unsigned char tail_frames;
    unsigned char tail_period;
    unsigned char tail_mode;
    unsigned char hold_frames;
    unsigned char hold_counter;
    unsigned char body_period;
    unsigned char body_mode;
} NoiseVoice;

typedef struct DmcSample {
    unsigned char rate;
    unsigned char start;
    unsigned char length;
    unsigned char level;
} DmcSample;

const DmcSample dmc_kick  = {0x0C, 0xC0, 0x34, 0x44};
const DmcSample dmc_snare = {0x0C, 0xCE, 0x1C, 0x42};
const DmcSample dmc_rim   = {0x0C, 0xD6, 0x0F, 0x3A};
const DmcSample dmc_voice = {0x0C, 0xDA, 0x94, 0x34};

unsigned char ctrl_to_vibrato_depth(unsigned char v);
unsigned char ctrl_to_vibrato_phase_step(unsigned char v);
unsigned char smooth_u8(unsigned char current, unsigned char target, unsigned char step);
unsigned char smooth_ctrl_with_deadzone(unsigned char current, unsigned char target, unsigned char deadzone, unsigned char step);
unsigned char nibble_to_ctrl(unsigned char v);
unsigned char fast_lfo_nibble_to_ctrl(unsigned char v);
unsigned char velocity_to_15(unsigned char vel);
unsigned char velocity_tier_to_level(unsigned char tier);
unsigned char ctrl_to_attack_rate(unsigned char v);
unsigned char ctrl_to_attack_start_level(unsigned char v);
unsigned char ctrl_to_decay_rate(unsigned char v);
unsigned char ctrl_to_release_rate(unsigned char v);
unsigned char ctrl_to_sustain(unsigned char v);
unsigned char effective_release_rate_for_volume(unsigned char release_rate, unsigned char env_volume);
unsigned char safe_ctrl_value(unsigned char previous, unsigned char incoming);
unsigned int ctrl_to_glide_step(unsigned char v);
unsigned int glide_timer_towards(unsigned int current, unsigned int target, unsigned int step);
unsigned int offset_timer(unsigned int base, signed char offset);
signed char vibrato_step(unsigned char depth_ctrl, unsigned char rate_ctrl, unsigned char* counter, unsigned char* phase);
unsigned char decode_lfo_target(unsigned char v);
unsigned char modulated_duty(unsigned char base_duty, signed char lfo_offset);
unsigned char modulated_level(unsigned char base_level, signed char lfo_offset);
void fast_apply_env_command(unsigned char command, unsigned char value);
void fast_pulse_env_step(unsigned char* phase, unsigned char* counter, unsigned char* volume, unsigned char attack_rate, unsigned char decay_rate, unsigned char sustain_level, unsigned char release_rate);

#if !CONTROLLER_RX_COMPACT
void controller_rx_read_packet(volatile unsigned char* dst, unsigned char size);
#endif
void controller_rx_read_parallel_packets(volatile unsigned char* dst0, volatile unsigned char* dst3, volatile unsigned char* dst4, unsigned char size);
void controller_rx_apply_compact_packet(void);
void controller_rx_fast_p2_loop(void);
void init_bus_defaults(void);
void controller_rx_delay(unsigned char count);
void controller_rx_sync_delay(void);
unsigned char controller_rx_read_bit(void);
const DmcSample* dmc_sample_from_note(unsigned char note);
void dmc_trigger(const DmcSample* sample);

volatile unsigned char controller_rx_compact[CONTROLLER_RX_COMPACT_SIZE];
volatile unsigned char controller_rx_compact_d3[CONTROLLER_RX_COMPACT_SIZE];
volatile unsigned char controller_rx_compact_d4[CONTROLLER_RX_COMPACT_SIZE];
// Supprimé: variables de diagnostic inutiles

void wait_vblank(void) {
    while (PPU_STATUS & 0x80) { }
    while (!(PPU_STATUS & 0x80)) { }
}

void ppu_set_addr(unsigned int addr) {
    PPU_STATUS;
    PPU_ADDR = (unsigned char)(addr >> 8);
    PPU_ADDR = (unsigned char)(addr & 0xFF);
}

unsigned char text_tile(char c) {
    if (c < 32 || c > 90) {
        return 0;
    }
    return (unsigned char)(c - 32);
}

void ppu_write_text(unsigned char x, unsigned char y, const char* text) {
    ppu_set_addr(0x2000U + ((unsigned int)y * 32U) + x);
    while (*text) {
        PPU_DATA = text_tile(*text++);
    }
}

void ppu_write_u8_3(unsigned char x, unsigned char y, unsigned char value) {
    unsigned char hundreds = value / 100;
    unsigned char tens = (value / 10) % 10;
    unsigned char ones = value % 10;

    ppu_set_addr(0x2000U + ((unsigned int)y * 32U) + x);
    PPU_DATA = hundreds ? (0x10 + hundreds) : 0;
    PPU_DATA = (hundreds || tens) ? (0x10 + tens) : 0;
    PPU_DATA = 0x10 + ones;
}

void ppu_write_note_field(unsigned char x, unsigned char y, unsigned char gate, unsigned char note) {
    ppu_set_addr(0x2000U + ((unsigned int)y * 32U) + x);
    PPU_DATA = gate ? text_tile('O') : text_tile('-');
    if (gate) {
        PPU_DATA = note >= 100 ? (0x10 + (note / 100)) : 0;
        PPU_DATA = note >= 10 ? (0x10 + ((note / 10) % 10)) : 0;
        PPU_DATA = 0x10 + (note % 10);
    } else {
        PPU_DATA = text_tile('-');
        PPU_DATA = text_tile('-');
        PPU_DATA = text_tile('-');
    }
}

void ppu_write_param_target(unsigned char x, unsigned char y, unsigned char command) {
    unsigned char param = (command >> 4) & 0x0F;
    unsigned char target = command & 0x0F;

    ppu_set_addr(0x2000U + ((unsigned int)y * 32U) + x);
    if (command == 0xFF) {
        PPU_DATA = text_tile('-');
        PPU_DATA = text_tile('-');
        PPU_DATA = text_tile('-');
    } else if (param == CONTROL_PARAM_ATTACK) {
        PPU_DATA = text_tile('A');
        PPU_DATA = text_tile('T');
        PPU_DATA = text_tile('K');
    } else if (param == CONTROL_PARAM_DECAY) {
        PPU_DATA = text_tile('D');
        PPU_DATA = text_tile('E');
        PPU_DATA = text_tile('C');
    } else if (param == CONTROL_PARAM_SUSTAIN) {
        PPU_DATA = text_tile('S');
        PPU_DATA = text_tile('U');
        PPU_DATA = text_tile('S');
    } else if (param == CONTROL_PARAM_RELEASE) {
        PPU_DATA = text_tile('R');
        PPU_DATA = text_tile('E');
        PPU_DATA = text_tile('L');
    } else {
        PPU_DATA = text_tile('?');
        PPU_DATA = text_tile('?');
        PPU_DATA = text_tile('?');
    }

    ppu_set_addr(0x2000U + ((unsigned int)y * 32U) + x + 8U);
    if (command == 0xFF) {
        PPU_DATA = text_tile('-');
        PPU_DATA = text_tile('-');
        PPU_DATA = text_tile('-');
    } else if (target == CONTROL_TARGET_P1) {
        PPU_DATA = text_tile('P');
        PPU_DATA = text_tile('1');
        PPU_DATA = 0;
    } else if (target == CONTROL_TARGET_P2) {
        PPU_DATA = text_tile('P');
        PPU_DATA = text_tile('2');
        PPU_DATA = 0;
    } else if (target == CONTROL_TARGET_GLOBAL) {
        PPU_DATA = text_tile('G');
        PPU_DATA = text_tile('L');
        PPU_DATA = text_tile('B');
    } else {
        PPU_DATA = text_tile('-');
        PPU_DATA = text_tile('-');
        PPU_DATA = text_tile('-');
    }
}

unsigned char read_joy1(void) {
    unsigned char i;
    unsigned char buttons = 0;

    JOY_STROBE = 1;
    controller_rx_delay(8);
    JOY_STROBE = 0;
    controller_rx_delay(8);

    for (i = 0; i < 8; ++i) {
        if (JOY1_PORT & 0x01) {
            buttons |= (1 << i);
        }
        controller_rx_delay(8);
    }

    return buttons;
}

void ppu_boot_init(void) {
    unsigned int i;

    PPU_CTRL = 0x00;
    PPU_MASK = 0x00;
    wait_vblank();

    ppu_set_addr(0x3F00U);
    PPU_DATA = 0x0F;
    PPU_DATA = 0x30;
    PPU_DATA = 0x16;
    PPU_DATA = 0x00;
    for (i = 4; i < 32; ++i) {
        PPU_DATA = 0x0F;
    }

    ppu_set_addr(0x2000U);
    for (i = 0; i < 1024; ++i) {
        PPU_DATA = 0;
    }

    ppu_write_text(3, 2,  "==========================");
    ppu_write_text(12, 4, "APU-8");
    ppu_write_text(6, 5,  "NES MIDI ENGINE");
    ppu_write_text(3, 7,  "==========================");
    ppu_write_text(3, 9,  "LINK");
    ppu_write_text(3, 10, "D0:0  OK:000  BAD:000");
    ppu_write_text(3, 12, "VOICES");
    ppu_write_text(3, 13, "P1:----    P2:----");
    ppu_write_text(3, 14, "TR:----    NS:----");
    ppu_write_text(3, 16, "CONTROL");
    ppu_write_text(3, 17, "PARAM:---  TO:---");
    ppu_write_text(3, 18, "VALUE:000");
    ppu_write_text(3, 19, "A1:000 A2:000");
    // Supprimé: tout le code de diagnostic inutile

    // Réactiver le PPU pour l'affichage
    PPU_CTRL = 0x90;  // NMI on + pattern table 0
    PPU_MASK = 0x1E;  // Show background + sprites
}

void controller_rx_delay(unsigned char count) {
    volatile unsigned char i;
    for (i = 0; i < count; ++i) {
    }
}

void controller_rx_sync_delay(void) {
    unsigned char i;
    for (i = 0; i < 3; ++i) {
        controller_rx_delay(240);
    }
}

void controller_rx_latch(void) {
    JOY_STROBE = 1;
    controller_rx_sync_delay();
    JOY_STROBE = 0;
    controller_rx_delay(16);
}

void controller_rx_advance(void) {
    JOY_STROBE = 1;
    controller_rx_delay(16);
    JOY_STROBE = 0;
    controller_rx_delay(16);
}

void controller_rx_arm(void) {
    unsigned char i;
    for (i = 0; i < 3; ++i) {
        JOY_STROBE = 1;
        controller_rx_sync_delay();
        JOY_STROBE = 0;
        controller_rx_sync_delay();
    }
}

unsigned char controller_rx_read_bit(void) {
    unsigned char bit;

#if CONTROLLER_RX_PORT == 2
    bit = JOY2_PORT & CONTROLLER_RX_MASK;
#else
    bit = JOY1_PORT & CONTROLLER_RX_MASK;
#endif

    bit = bit ? 1 : 0;

#if CONTROLLER_RX_INVERT
    bit ^= 1;
#endif

    return bit;
}

#if !CONTROLLER_RX_COMPACT
void controller_rx_read_packet(volatile unsigned char* dst, unsigned char size) {
    unsigned char i;
    unsigned char mask;
    unsigned char value;

    controller_rx_latch();

    for (i = 0; i < size; ++i) {
        value = 0;
        mask = 1;
        while (mask) {
            if (controller_rx_read_bit()) {
                value |= mask;
            }
            controller_rx_advance();
            controller_rx_delay(4);
            mask <<= 1;
        }
        dst[i] = value;
    }
}
#endif

void controller_rx_read_parallel_packets(volatile unsigned char* dst0, volatile unsigned char* dst3, volatile unsigned char* dst4, unsigned char size) {
    unsigned char i;
    unsigned char mask;
    unsigned char value0;
    unsigned char value3;
    unsigned char value4;
    unsigned char port;

    controller_rx_latch();

    for (i = 0; i < size; ++i) {
        value0 = 0;
        value3 = 0;
        value4 = 0;
        mask = 1;
        while (mask) {
#if CONTROLLER_RX_PORT == 2
            port = JOY2_PORT;
#else
            port = JOY1_PORT;
#endif
            // Slightly longer settle for DN4 testing, while staying short enough for D0/D3.
            __asm__("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
            if (port & CONTROLLER_RX_MASK) {
                value0 |= mask;
            }
            if (port & CONTROLLER_RX_D3_MASK) {
                value3 |= mask;
            }
            if (port & CONTROLLER_RX_D4_MASK) {
                value4 |= mask;
            }
            controller_rx_advance();
            controller_rx_delay(4);
            mask <<= 1;
        }
#if CONTROLLER_RX_INVERT
        value0 ^= 0xFF;
#endif
#if CONTROLLER_RX_D3_INVERT
        value3 ^= 0xFF;
#endif
#if CONTROLLER_RX_D4_INVERT
        value4 ^= 0xFF;
#endif
        dst0[i] = value0;
        dst3[i] = value3;
        dst4[i] = value4;
    }
}

void init_bus_defaults(void) {
    unsigned char i;
    for (i = 0; i < BUS_PACKET_SIZE; ++i) {
        BUS_BASE[i] = 0;
    }

    BUS_MAGIC_A = 0xA8;
    BUS_MAGIC_B = 0x58;
    BUS_P1_DUTY = 2;
    BUS_P2_DUTY = 2;

    BUS_P1_ATTACK = 0;
    BUS_P1_DECAY = 0;
    BUS_P1_SUSTAIN = 127;
    BUS_P1_RELEASE = 0;
    BUS_P2_ATTACK = 0;
    BUS_P2_DECAY = 0;
    BUS_P2_SUSTAIN = 127;
    BUS_P2_RELEASE = 0;
    BUS_TRI_ATTACK = 0;
    BUS_TRI_DECAY = 0;
    BUS_TRI_SUSTAIN = 127;
    BUS_TRI_RELEASE = 0;
    BUS_NOI_ATTACK = 0;
    BUS_NOI_DECAY = 0;
    BUS_NOI_SUSTAIN = 127;
    BUS_NOI_RELEASE = 0;
}

void controller_rx_apply_compact_packet(void) {
    controller_rx_read_parallel_packets(controller_rx_compact, controller_rx_compact_d3, controller_rx_compact_d4, CONTROLLER_RX_COMPACT_SIZE);

#if CONTROLLER_RX_FAST_P2
    if (controller_rx_compact[0] != 0xA8 ||
        (controller_rx_compact[1] != 0x58 && controller_rx_compact[1] != CONTROL_PACKET_MAGIC)) {
        return;
    }

    BUS_P1_TRIG = controller_rx_compact[2];
    BUS_P1_NOTE = controller_rx_compact[3] & 0x7F;
    BUS_P1_GATE = (controller_rx_compact[3] & 0x80) ? 1 : 0;
    BUS_P2_TRIG = controller_rx_compact[4];
    BUS_P2_NOTE = controller_rx_compact[5] & 0x7F;
    BUS_P2_GATE = (controller_rx_compact[5] & 0x80) ? 1 : 0;

    if (controller_rx_compact[1] == CONTROL_PACKET_MAGIC) {
        BUS_NOI_GATE = 0;
        // ADSR maintenant lu depuis D3 uniquement, pas depuis D0
        // BUS_P1_ATTACK = nibble_to_ctrl((controller_rx_compact[6] >> 4) & 0x0F);
        // BUS_P2_ATTACK = nibble_to_ctrl(controller_rx_compact[6] & 0x0F);
        // BUS_P1_DECAY = nibble_to_ctrl((controller_rx_compact[7] >> 4) & 0x0F);
        // BUS_P2_DECAY = nibble_to_ctrl(controller_rx_compact[7] & 0x0F);
        // BUS_P1_SUSTAIN = nibble_to_ctrl((controller_rx_compact[8] >> 4) & 0x0F);
        // BUS_P2_SUSTAIN = nibble_to_ctrl(controller_rx_compact[8] & 0x0F);
        BUS_P1_VIB_DEPTH = fast_lfo_nibble_to_ctrl(controller_rx_compact[9] & 0x0F);
        BUS_P1_VIB_RATE = nibble_to_ctrl((controller_rx_compact[9] >> 4) & 0x0F);
        BUS_P2_VIB_DEPTH = fast_lfo_nibble_to_ctrl((controller_rx_compact[10] >> 4) & 0x0F);
        BUS_P2_VIB_RATE = nibble_to_ctrl((controller_rx_compact[11] >> 4) & 0x0F);
        BUS_P1_LFO_TARGET = LFO_TARGET_PITCH;
        BUS_P2_LFO_TARGET = LFO_TARGET_PITCH;
        BUS_P1_VEL = velocity_tier_to_level((controller_rx_compact[10] >> 2) & 0x03);
        BUS_P2_VEL = velocity_tier_to_level((controller_rx_compact[11] >> 2) & 0x03);
        // SUSTAIN maintenant lu depuis D3
        // BUS_P1_SUSTAIN = 127;
        // BUS_P2_SUSTAIN = 127;
    } else {
        BUS_TRI_TRIG = controller_rx_compact[6];
        BUS_TRI_NOTE = controller_rx_compact[7] & 0x7F;
        BUS_TRI_VEL = 110;
        BUS_TRI_GATE = (controller_rx_compact[7] & 0x80) ? 1 : 0;

        if (controller_rx_compact[8] == CONTROL_COMMAND_TRIGGER &&
            controller_rx_compact[9] == CONTROL_COMMAND_MAGIC) {
            fast_apply_env_command(controller_rx_compact[10], controller_rx_compact[11]);
        } else {
            BUS_NOI_TRIG = controller_rx_compact[8];
            BUS_NOI_NOTE = controller_rx_compact[9] & 0x7F;
            BUS_NOI_VEL = 110;
            BUS_NOI_GATE = (controller_rx_compact[9] & 0x80) ? 1 : 0;
            BUS_P1_VEL = (controller_rx_compact[10] >> 4) & 0x0F;
            BUS_P2_VEL = (controller_rx_compact[11] >> 4) & 0x0F;
        }
    }

    if (controller_rx_compact_d3[0] == 0xD3 && controller_rx_compact_d3[1] == 0x3D) {
        BUS_P1_ATTACK = nibble_to_ctrl((controller_rx_compact_d3[2] >> 4) & 0x0F);
        BUS_P2_ATTACK = nibble_to_ctrl(controller_rx_compact_d3[2] & 0x0F);
        BUS_P1_DECAY = nibble_to_ctrl((controller_rx_compact_d3[3] >> 4) & 0x0F);
        BUS_P2_DECAY = nibble_to_ctrl(controller_rx_compact_d3[3] & 0x0F);
        BUS_P1_SUSTAIN = nibble_to_ctrl((controller_rx_compact_d3[4] >> 4) & 0x0F);
        BUS_P2_SUSTAIN = nibble_to_ctrl(controller_rx_compact_d3[4] & 0x0F);
        BUS_P1_RELEASE = nibble_to_ctrl((controller_rx_compact_d3[5] >> 4) & 0x0F);
        BUS_P2_RELEASE = nibble_to_ctrl(controller_rx_compact_d3[5] & 0x0F);
        BUS_P1_DUTY = (controller_rx_compact_d3[6] >> 4) & 0x03;
        BUS_P2_DUTY = controller_rx_compact_d3[6] & 0x03;
    }
    if ((controller_rx_compact_d4[0] == 0xD4 && controller_rx_compact_d4[1] == 0x4D) ||
        (controller_rx_compact_d4[0] == 0x2B && controller_rx_compact_d4[1] == 0xB2) ||
        controller_rx_compact_d4[2] != 0 || controller_rx_compact_d4[3] != 0 ||
        controller_rx_compact_d4[4] != 0 || controller_rx_compact_d4[5] != 0 ||
        controller_rx_compact_d4[6] != 0) {
        BUS_P1_VIB_DEPTH = fast_lfo_nibble_to_ctrl((controller_rx_compact_d4[2] >> 4) & 0x0F);
        BUS_P2_VIB_DEPTH = fast_lfo_nibble_to_ctrl(controller_rx_compact_d4[2] & 0x0F);
        BUS_P1_VIB_RATE = nibble_to_ctrl((controller_rx_compact_d4[3] >> 4) & 0x0F);
        BUS_P2_VIB_RATE = nibble_to_ctrl(controller_rx_compact_d4[3] & 0x0F);
        BUS_P1_LFO_TARGET = (controller_rx_compact_d4[4] >> 4) & 0x03;
        BUS_P2_LFO_TARGET = controller_rx_compact_d4[4] & 0x03;
        BUS_TRI_VIB_DEPTH = fast_lfo_nibble_to_ctrl((controller_rx_compact_d4[5] >> 4) & 0x0F);
        BUS_TRI_VIB_RATE = nibble_to_ctrl(controller_rx_compact_d4[5] & 0x0F);
        BUS_TRI_LFO_TARGET = controller_rx_compact_d4[6] & 0x03;
    }
    return;  // FAST_P2: traitement complet, ne pas exécuter le code de validation en double ci-dessous
#endif

    // Validation sans inversion (INVERT=0): attendre 0xA8 et 0x5A
    if (controller_rx_compact[0] != 0xA8 || controller_rx_compact[1] != 0x5A) {
        BUS_P1_GATE = 0;
        BUS_P2_GATE = 0;
        BUS_TRI_GATE = 0;
        BUS_NOI_GATE = 0;
        return;
    }

    BUS_SEQ = controller_rx_compact[0];
    BUS_MAGIC_A = controller_rx_compact[1];
    BUS_MAGIC_B = controller_rx_compact[2];

    // Format 12 bytes compact: P1, P2, TRI
    // [0]=0xA8→0x57, [1]=0x5A→0xA5, [2]=triggers, [3]=P1, [5]=P2, [7]=TRI
    
    BUS_P1_TRIG = controller_rx_compact[2];
    BUS_P1_NOTE = controller_rx_compact[3] & 0x7F;
    BUS_P1_GATE = (controller_rx_compact[3] & 0x80) ? 1 : 0;
    BUS_P1_VEL = 110;
    
    BUS_P2_TRIG = controller_rx_compact[4];
    BUS_P2_NOTE = controller_rx_compact[5] & 0x7F;
    BUS_P2_GATE = (controller_rx_compact[5] & 0x80) ? 1 : 0;
    BUS_P2_VEL = 110;
    
    // TRI en index 7 (compatible avec writeVoicePacketFromState du sketch)
    BUS_TRI_TRIG = controller_rx_compact[6];
    BUS_TRI_NOTE = controller_rx_compact[7] & 0x7F;
    BUS_TRI_GATE = (controller_rx_compact[7] & 0x80) ? 1 : 0;
    BUS_TRI_VEL = 110;
    
    // NOISE désactivé (pour plus tard)
    BUS_NOI_GATE = 0;
    BUS_NOI_TRIG = 0;
}

unsigned int pulse_timer_from_note(unsigned char midi) {
    if (midi >= 12) {
        midi -= 12;
    } else {
        midi = 0;
    }
    if (midi < 24) midi = 24;
    if (midi > 84) midi = 84;
    return pulse_timer_table[midi - 24];
}

unsigned int triangle_timer_from_note(unsigned char midi) {
    return pulse_timer_from_note(midi) >> 1;
}

unsigned char noise_period_from_note(unsigned char midi) {
    return noise_period_table[(midi >> 3) & 0x0F];
}

void fast_apply_env_command(unsigned char command, unsigned char value) {
    unsigned char target = command & 0x0F;
    unsigned char param = (command >> 4) & 0x0F;

    if (target == CONTROL_TARGET_P1 || target == CONTROL_TARGET_GLOBAL) {
        if (param == CONTROL_PARAM_ATTACK) {
            BUS_P1_ATTACK = value;
        } else if (param == CONTROL_PARAM_DECAY) {
            BUS_P1_DECAY = value;
        } else if (param == CONTROL_PARAM_SUSTAIN) {
            BUS_P1_SUSTAIN = value;
        } else if (param == CONTROL_PARAM_RELEASE) {
            BUS_P1_RELEASE = value;
        }
    }

    if (target == CONTROL_TARGET_P2 || target == CONTROL_TARGET_GLOBAL) {
        if (param == CONTROL_PARAM_ATTACK) {
            BUS_P2_ATTACK = value;
        } else if (param == CONTROL_PARAM_DECAY) {
            BUS_P2_DECAY = value;
        } else if (param == CONTROL_PARAM_SUSTAIN) {
            BUS_P2_SUSTAIN = value;
        } else if (param == CONTROL_PARAM_RELEASE) {
            BUS_P2_RELEASE = value;
        }
    }
}

void fast_pulse_env_step(unsigned char* phase, unsigned char* counter, unsigned char* volume, unsigned char attack_rate, unsigned char decay_rate, unsigned char sustain_level, unsigned char release_rate) {
    unsigned char effective_release_rate;

    switch (*phase) {
        case ENV_OFF:
            return;
        case ENV_ATTACK:
            if (attack_rate == 0) {
                *volume = FAST_PULSE_MAX_LEVEL;
                *phase = ENV_DECAY;
                *counter = 0;
                return;
            }
            ++(*counter);
            if (*counter >= attack_rate) {
                *counter = 0;
                if (*volume < FAST_PULSE_MAX_LEVEL) {
                    ++(*volume);
                } else {
                    *phase = ENV_DECAY;
                }
            }
            return;
        case ENV_DECAY:
            if (decay_rate == 0) {
                *volume = sustain_level;
                *phase = ENV_SUSTAIN;
                *counter = 0;
                return;
            }
            ++(*counter);
            if (*counter >= decay_rate) {
                *counter = 0;
                if (*volume > sustain_level) {
                    --(*volume);
                } else {
                    *phase = ENV_SUSTAIN;
                }
            }
            return;
        case ENV_SUSTAIN:
            *volume = sustain_level;
            return;
        case ENV_RELEASE:
            if (release_rate == 0) {
                *volume = 0;
                *phase = ENV_OFF;
                *counter = 0;
                return;
            }
            effective_release_rate = effective_release_rate_for_volume(release_rate, *volume);
            ++(*counter);
            if (*counter >= effective_release_rate) {
                *counter = 0;
                if (*volume > 0) {
                    --(*volume);
                } else {
                    *phase = ENV_OFF;
                }
            }
            return;
    }
}

void controller_rx_fast_p2_loop(void) {
    unsigned char p1_last_gate = 0xFF;
    unsigned char p1_last_note = 0xFF;
    unsigned char p1_last_trigger = 0xFF;
    unsigned char p2_last_gate = 0xFF;
    unsigned char p2_last_note = 0xFF;
    unsigned char p2_last_trigger = 0xFF;
    unsigned char tri_last_gate = 0xFF;
    unsigned char tri_last_note = 0xFF;
    unsigned char tri_last_trigger = 0xFF;
    unsigned char noi_last_gate = 0xFF;
    unsigned char noi_last_note = 0xFF;
    unsigned char noi_last_trigger = 0xFF;
    unsigned char p1_last_level = 0xFF;
    unsigned char p2_last_level = 0xFF;
    unsigned char p1_last_duty = 0xFF;
    unsigned char p2_last_duty = 0xFF;
    unsigned char p1_last_lo = 0xFF;
    unsigned char p1_last_hi = 0xFF;
    unsigned char p2_last_lo = 0xFF;
    unsigned char p2_last_hi = 0xFF;
    unsigned char tri_last_lo = 0xFF;
    unsigned char tri_last_hi = 0xFF;
    unsigned char p1_env_phase = ENV_OFF;
    unsigned char p1_env_counter = 0;
    unsigned char p1_env_volume = 0;
    unsigned char p2_env_phase = ENV_OFF;
    unsigned char p2_env_counter = 0;
    unsigned char p2_env_volume = 0;
    unsigned char p1_vibrato_counter = 0;
    unsigned char p1_vibrato_phase = 0;
    unsigned char p2_vibrato_counter = 0;
    unsigned char p2_vibrato_phase = 0;
    signed char p1_vibrato_offset = 0;
    signed char p2_vibrato_offset = 0;
    unsigned char p1_lfo_active = 0;
    unsigned char p2_lfo_active = 0;
    unsigned char p1_lfo_now_active;
    unsigned char p2_lfo_now_active;
    unsigned char p1_lfo_tick;
    unsigned char p2_lfo_tick;
    unsigned char duty;
    unsigned char level;
    unsigned char period;
    unsigned char mode;
    unsigned char lo;
    unsigned char hi;
    unsigned char new_note;
    unsigned char release_started;
    unsigned char packet_seen;
    unsigned char env_tick;
    unsigned char p1_sustain_level;
    unsigned char p2_sustain_level;
    const DmcSample* dmc_sample;
    unsigned int timer;
    unsigned int env_counter = 0;

    while (1) {
        packet_seen = 0;
        env_tick = 0;
        p1_lfo_tick = 0;
        p2_lfo_tick = 0;

        if (controller_rx_read_bit()) {
            controller_rx_apply_compact_packet();
            packet_seen = 1;
        } else {
#if CONTROLLER_RX_FAST_IDLE_DELAY > 0
            controller_rx_delay(CONTROLLER_RX_FAST_IDLE_DELAY);
#endif
        }

        ++env_counter;
        if (env_counter >= FAST_ENV_TICK_COUNT) {
            env_counter = 0;
            env_tick = 1;
            p1_lfo_tick = 1;
            p2_lfo_tick = 1;
        }

        p1_lfo_now_active = (BUS_P1_VIB_DEPTH > 0 && BUS_P1_VIB_RATE > 0) ? 1 : 0;
        p2_lfo_now_active = (BUS_P2_VIB_DEPTH > 0 && BUS_P2_VIB_RATE > 0) ? 1 : 0;
        if (p1_lfo_now_active && !p1_lfo_active) {
            p1_vibrato_phase = 0;
            p1_lfo_tick = 1;
        }
        if (p2_lfo_now_active && !p2_lfo_active) {
            p2_vibrato_phase = 0;
            p2_lfo_tick = 1;
        }
        if (packet_seen && BUS_P1_GATE) {
            p1_lfo_tick = 1;
        }
        if (packet_seen && BUS_P2_GATE) {
            p2_lfo_tick = 1;
        }
        p1_lfo_active = p1_lfo_now_active;
        p2_lfo_active = p2_lfo_now_active;

        if (!packet_seen && !env_tick && !p1_lfo_tick && !p2_lfo_tick) {
            continue;
        }

        if (p1_lfo_tick && p1_lfo_active) {
            p1_vibrato_offset = vibrato_step(BUS_P1_VIB_DEPTH, BUS_P1_VIB_RATE, &p1_vibrato_counter, &p1_vibrato_phase);
        } else if (!p1_lfo_active) {
            p1_vibrato_offset = 0;
            p1_vibrato_phase = 0;
        }
        if (p2_lfo_tick && p2_lfo_active) {
            p2_vibrato_offset = vibrato_step(BUS_P2_VIB_DEPTH, BUS_P2_VIB_RATE, &p2_vibrato_counter, &p2_vibrato_phase);
        } else if (!p2_lfo_active) {
            p2_vibrato_offset = 0;
            p2_vibrato_phase = 0;
        }

        release_started = 0;
        new_note = (BUS_P1_GATE && (BUS_P1_NOTE != p1_last_note || BUS_P1_GATE != p1_last_gate || BUS_P1_TRIG != p1_last_trigger));
        if (new_note) {
            p1_env_phase = ENV_ATTACK;
            p1_env_counter = 0;
            p1_env_volume = ctrl_to_attack_start_level(BUS_P1_ATTACK);
            p1_last_lo = 0xFF;
            p1_last_hi = 0xFF;
        } else if (!BUS_P1_GATE && p1_last_gate != 0 && p1_env_phase != ENV_OFF) {
            p1_env_phase = ENV_RELEASE;
            p1_env_counter = 0;
            release_started = 1;
        }

        if ((env_tick || new_note || release_started) && p1_env_phase != ENV_OFF) {
            p1_sustain_level = ctrl_to_sustain(BUS_P1_SUSTAIN);
            fast_pulse_env_step(&p1_env_phase, &p1_env_counter, &p1_env_volume,
                                ctrl_to_attack_rate(BUS_P1_ATTACK),
                                ctrl_to_decay_rate(BUS_P1_DECAY),
                                p1_sustain_level,
                                ctrl_to_release_rate(BUS_P1_RELEASE));
        }

        if (p1_env_phase != ENV_OFF) {
            duty = BUS_P1_DUTY & 0x03;
            level = p1_env_volume & 0x0F;
            if (level > FAST_PULSE_MAX_LEVEL) {
                level = FAST_PULSE_MAX_LEVEL;
            }
            timer = offset_timer(pulse_timer_from_note(BUS_P1_NOTE), p1_vibrato_offset);
            lo = timer & 0xFF;
            hi = ((timer >> 8) & 0x07) | 0xF8;

            if (level != p1_last_level || duty != p1_last_duty) {
                PULSE1_VOL = (duty << 6) | 0x30 | level;
                PULSE1_SWEEP = 0x00;
                p1_last_level = level;
                p1_last_duty = duty;
            }

            if (lo != p1_last_lo || new_note) {
                PULSE1_LO = lo;
                p1_last_lo = lo;
            }
            if (hi != p1_last_hi || new_note) {
                PULSE1_HI = hi;
                p1_last_hi = hi;
                p1_last_note = BUS_P1_NOTE;
                p1_last_trigger = BUS_P1_TRIG;
            }
        } else {
            if (p1_last_level != 0xFF) {
                PULSE1_VOL = 0x30;
            }
            p1_last_level = 0xFF;
            p1_last_duty = 0xFF;
            p1_last_lo = 0xFF;
            p1_last_hi = 0xFF;
        }

        release_started = 0;
        new_note = (BUS_P2_GATE && (BUS_P2_NOTE != p2_last_note || BUS_P2_GATE != p2_last_gate || BUS_P2_TRIG != p2_last_trigger));
        if (new_note) {
            p2_env_phase = ENV_ATTACK;
            p2_env_counter = 0;
            p2_env_volume = ctrl_to_attack_start_level(BUS_P2_ATTACK);
            p2_last_lo = 0xFF;
            p2_last_hi = 0xFF;
        } else if (!BUS_P2_GATE && p2_last_gate != 0 && p2_env_phase != ENV_OFF) {
            p2_env_phase = ENV_RELEASE;
            p2_env_counter = 0;
            release_started = 1;
        }

        if ((env_tick || new_note || release_started) && p2_env_phase != ENV_OFF) {
            p2_sustain_level = ctrl_to_sustain(BUS_P2_SUSTAIN);
            fast_pulse_env_step(&p2_env_phase, &p2_env_counter, &p2_env_volume,
                                ctrl_to_attack_rate(BUS_P2_ATTACK),
                                ctrl_to_decay_rate(BUS_P2_DECAY),
                                p2_sustain_level,
                                ctrl_to_release_rate(BUS_P2_RELEASE));
        }

        if (p2_env_phase != ENV_OFF) {
            duty = BUS_P2_DUTY & 0x03;
            level = p2_env_volume & 0x0F;
            if (level > FAST_PULSE_MAX_LEVEL) {
                level = FAST_PULSE_MAX_LEVEL;
            }
            timer = offset_timer(pulse_timer_from_note(BUS_P2_NOTE), p2_vibrato_offset);
            lo = timer & 0xFF;
            hi = ((timer >> 8) & 0x07) | 0xF8;

            if (level != p2_last_level || duty != p2_last_duty) {
                PULSE2_VOL = (duty << 6) | 0x30 | level;
                PULSE2_SWEEP = 0x00;
                p2_last_level = level;
                p2_last_duty = duty;
            }

            if (lo != p2_last_lo || new_note) {
                PULSE2_LO = lo;
                p2_last_lo = lo;
            }
            if (hi != p2_last_hi || new_note) {
                PULSE2_HI = hi;
                p2_last_hi = hi;
                p2_last_note = BUS_P2_NOTE;
                p2_last_trigger = BUS_P2_TRIG;
            }
        } else {
            if (p2_last_level != 0xFF) {
                PULSE2_VOL = 0x30;
            }
            p2_last_level = 0xFF;
            p2_last_duty = 0xFF;
            p2_last_lo = 0xFF;
            p2_last_hi = 0xFF;
        }

        if (packet_seen && BUS_TRI_GATE) {
            timer = triangle_timer_from_note(BUS_TRI_NOTE);
            lo = timer & 0xFF;
            hi = (timer >> 8) & 0x07;
            new_note = (BUS_TRI_NOTE != tri_last_note || BUS_TRI_GATE != tri_last_gate || BUS_TRI_TRIG != tri_last_trigger);
            if (new_note) {
                TRI_LINEAR = 0x80 | 0x7F;
            }
            if (lo != tri_last_lo || new_note) {
                TRI_LO = lo;
                tri_last_lo = lo;
            }
            if (hi != tri_last_hi || new_note) {
                TRI_HI = hi;
                tri_last_hi = hi;
                tri_last_note = BUS_TRI_NOTE;
                tri_last_trigger = BUS_TRI_TRIG;
            }
        } else if (packet_seen) {
            if (tri_last_gate != 0) {
                TRI_LINEAR = 0x00;
            }
            tri_last_lo = 0xFF;
            tri_last_hi = 0xFF;
        }

        if (packet_seen && BUS_NOI_GATE) {
            if (BUS_NOI_NOTE != noi_last_note || BUS_NOI_GATE != noi_last_gate || BUS_NOI_TRIG != noi_last_trigger) {
                dmc_sample = dmc_sample_from_note(BUS_NOI_NOTE);
                if (dmc_sample != 0) {
                    NOISE_VOL = 0x30;
                    dmc_trigger(dmc_sample);
                } else {
                    mode = (BUS_NOI_NOTE >= 84) ? 0x80 : 0x00;
                    period = noise_period_from_note(BUS_NOI_NOTE);
                    if (BUS_NOI_NOTE >= 72) {
                        level = 1;
                    } else if (BUS_NOI_NOTE >= 60) {
                        level = 2;
                    } else if (BUS_NOI_NOTE >= 48) {
                        level = 3;
                    } else {
                        level = 5;
                    }
                    NOISE_VOL = level;       // Hardware envelope, no loop.
                    NOISE_LO = mode | period;
                    NOISE_HI = 0x18;
                }
                noi_last_note = BUS_NOI_NOTE;
                noi_last_trigger = BUS_NOI_TRIG;
            }
        } else if (packet_seen) {
            if (noi_last_gate != 0) {
                NOISE_VOL = 0x30;
            }
        }

        p1_last_gate = BUS_P1_GATE;
        p2_last_gate = BUS_P2_GATE;
        tri_last_gate = BUS_TRI_GATE;
        noi_last_gate = BUS_NOI_GATE;

    }
}

unsigned char noise_period_from_timbre(unsigned char v) {
    return noise_period_table[(127 - v) >> 3];
}

unsigned char clamp_nibble(int v) {
    if (v < 0) {
        return 0;
    }
    if (v > 15) {
        return 15;
    }
    return (unsigned char)v;
}

void noise_voice_configure_drum(NoiseVoice* voice, unsigned char note, unsigned char timbre_ctrl, unsigned char mode_ctrl) {
    unsigned char color = timbre_ctrl >> 5;
    unsigned char metallic = mode_ctrl >= 96 ? 0x80 : 0x00;
    unsigned char variant = note & 0x03;
    unsigned char spread = note & 0x07;

    if (note < 38) {
        voice->drum_kind = DRUM_KICK;
        voice->transient_frames = 1;
        voice->transient_period = clamp_nibble(2 + variant + (color >> 1));
        voice->transient_mode = 0x00;
        voice->tail_frames = 1;
        voice->tail_period = clamp_nibble(7 + variant + color);
        voice->tail_mode = 0x00;
        voice->hold_frames = 2;
        voice->body_period = clamp_nibble(11 + variant + color);
        voice->body_mode = 0x00;
    } else if (note < 41) {
        voice->drum_kind = DRUM_SNARE;
        voice->transient_frames = 1;
        voice->transient_period = clamp_nibble(1 + variant + (color >> 1));
        voice->transient_mode = metallic;
        voice->tail_frames = 1;
        voice->tail_period = clamp_nibble(2 + variant + (color >> 1));
        voice->tail_mode = 0x00;
        voice->hold_frames = 1;
        voice->body_period = clamp_nibble(4 + variant + color);
        voice->body_mode = 0x00;
    } else if (note < 42) {
        voice->drum_kind = DRUM_SNARE;
        voice->transient_frames = 1;
        voice->transient_period = clamp_nibble(color >> 1);
        voice->transient_mode = metallic;
        voice->tail_frames = 0;
        voice->tail_period = clamp_nibble(1 + (color >> 1));
        voice->tail_mode = 0x00;
        voice->hold_frames = 1;
        voice->body_period = clamp_nibble(3 + color);
        voice->body_mode = 0x00;
    } else if (note < 48) {
        voice->drum_kind = DRUM_TOM;
        voice->transient_frames = 1;
        voice->transient_period = clamp_nibble(3 + (spread >> 2) + (color >> 1));
        voice->transient_mode = 0x00;
        voice->tail_frames = 1;
        voice->tail_period = clamp_nibble(6 + (spread >> 1) + color);
        voice->tail_mode = 0x00;
        voice->hold_frames = 2;
        voice->body_period = clamp_nibble(9 + spread + color);
        voice->body_mode = 0x00;
    } else if (note < 60) {
        voice->drum_kind = DRUM_SNARE;
        voice->transient_frames = 1;
        voice->transient_period = clamp_nibble((spread >> 2) + (color >> 1));
        voice->transient_mode = 0x00;
        voice->tail_frames = 2;
        voice->tail_period = clamp_nibble(2 + (spread >> 2) + (color >> 1));
        voice->tail_mode = 0x00;
        voice->hold_frames = 1;
        voice->body_period = clamp_nibble(4 + (spread >> 1) + color);
        voice->body_mode = 0x00;
    } else if (note < 72) {
        voice->drum_kind = DRUM_CLHAT;
        voice->transient_frames = 1;
        voice->transient_period = clamp_nibble((spread >> 2) + (color >> 2));
        voice->transient_mode = 0x00;
        voice->tail_frames = 0;
        voice->tail_period = clamp_nibble(1 + (spread >> 2) + (color >> 1));
        voice->tail_mode = metallic;
        voice->hold_frames = 0;
        voice->body_period = clamp_nibble(2 + (spread >> 1) + (color >> 1));
        voice->body_mode = 0x00;
    } else if (note < 84) {
        voice->drum_kind = DRUM_OPHAT;
        voice->transient_frames = 1;
        voice->transient_period = clamp_nibble((spread >> 2) + (color >> 2));
        voice->transient_mode = 0x00;
        voice->tail_frames = 0;
        voice->tail_period = clamp_nibble(2 + (spread >> 2) + (color >> 1));
        voice->tail_mode = 0x00;
        voice->hold_frames = 0;
        voice->body_period = clamp_nibble(3 + (spread >> 1) + (color >> 1));
        voice->body_mode = metallic;
    } else {
        voice->drum_kind = DRUM_CYMBAL;
        voice->transient_frames = 1;
        voice->transient_period = clamp_nibble(1 + (spread >> 2) + (color >> 2));
        voice->transient_mode = 0x00;
        voice->tail_frames = 0;
        voice->tail_period = clamp_nibble(3 + (spread >> 2) + (color >> 1));
        voice->tail_mode = metallic;
        voice->hold_frames = 0;
        voice->body_period = clamp_nibble((note >= 96 ? 6 : 4) + (spread >> 1) + color);
        voice->body_mode = note >= 96 ? 0x00 : metallic;
    }
}

unsigned char noise_shaped_rate(unsigned char base, unsigned char drum_kind, unsigned char is_release) {
    if (base == 0) {
        return 0;
    }

    switch (drum_kind) {
        case DRUM_KICK:
            return base > (is_release ? 4 : 2) ? (unsigned char)(base - (is_release ? 4 : 2)) : 1;
        case DRUM_TOM:
            return base > (is_release ? 3 : 1) ? (unsigned char)(base - (is_release ? 3 : 1)) : 1;
        case DRUM_SNARE:
            return base > (is_release ? 9 : 7) ? (unsigned char)(base - (is_release ? 9 : 7)) : 1;
        case DRUM_CLHAT:
            return base > (is_release ? 12 : 13) ? (unsigned char)(base - (is_release ? 12 : 13)) : 1;
        case DRUM_OPHAT:
            return base > (is_release ? 10 : 11) ? (unsigned char)(base - (is_release ? 10 : 11)) : 1;
        case DRUM_CYMBAL:
            return base > (is_release ? 8 : 9) ? (unsigned char)(base - (is_release ? 8 : 9)) : 1;
    }

    return base;
}

unsigned char noise_note_length_add(unsigned char note, unsigned char drum_kind, unsigned char is_release) {
    switch (drum_kind) {
        case DRUM_KICK:
            return is_release ? 2 : 1;
        case DRUM_TOM:
            return is_release ? 4 : 3;
        case DRUM_SNARE:
            if (note < 48) {
                return is_release ? 2 : 1;
            }
            return 0;
        case DRUM_CLHAT:
            return 0;
        case DRUM_OPHAT:
            return 0;
        case DRUM_CYMBAL:
            return 0;
    }

    return 0;
}

const DmcSample* dmc_sample_from_note(unsigned char note) {
    if (note >= 36 && note <= 37) {
        return &dmc_kick;
    }
    if (note >= 38 && note <= 40) {
        return &dmc_snare;
    }
    if (note == 41) {
        return &dmc_rim;
    }
    if (note == 46) {
        return &dmc_voice;
    }
    return 0;
}

void dmc_trigger(const DmcSample* sample) {
    APU_CTRL = APU_ENABLE_STD;
    DMC_FREQ = sample->rate & 0x0F;
    DMC_RAW = sample->level & 0x7F;
    DMC_START = sample->start;
    DMC_LEN = sample->length;
    APU_CTRL = APU_ENABLE_DMC;
}

void noise_voice_silence(NoiseVoice* voice) {
    voice->gate = 0;
    voice->env_phase = ENV_OFF;
    voice->env_counter = 0;
    voice->env_volume = 0;
    voice->transient_frames = 0;
    voice->tail_frames = 0;
    voice->hold_counter = 0;
    voice->last_period = 0xFF;
}

unsigned char ctrl_to_rate(unsigned char v) {
    if (v == 0) {
        return 0;
    }
    return 1 + (v >> 3);
}

unsigned char ctrl_to_attack_rate(unsigned char v) {
    if (v < 10) {
        return 0;
    }
    if (v < 30) return 1;
    if (v < 60) return 2;
    if (v < 90) return 3;
    return 4;
}

unsigned char ctrl_to_attack_start_level(unsigned char v) {
    if (v < 10) {
        return FAST_PULSE_MAX_LEVEL;
    }
    if (v < 30) return 12;
    if (v < 60) return 10;
    if (v < 90) return 8;
    return 6;
}

unsigned char ctrl_to_decay_rate(unsigned char v) {
    if (v < 10) {
        return 0;
    }
    if (v < 30) return 1;
    if (v < 50) return 2;
    if (v < 70) return 3;
    if (v < 90) return 5;
    if (v < 110) return 8;
    return 12;
}

unsigned char ctrl_to_release_rate(unsigned char v) {
    if (v < 10) {
        return 0;
    }
    if (v < 30) return 1;
    if (v < 50) return 2;
    if (v < 70) return 3;
    if (v < 90) return 5;
    if (v < 110) return 8;
    return 12;
}

unsigned char effective_release_rate_for_volume(unsigned char release_rate, unsigned char env_volume) {
    unsigned char effective_release_rate = release_rate;

    if (env_volume <= 2) {
        effective_release_rate += 2;
    } else if (env_volume <= 5) {
        effective_release_rate += 1;
    }

    return effective_release_rate;
}

unsigned char safe_ctrl_value(unsigned char previous, unsigned char incoming) {
    if (incoming > previous && (unsigned char)(incoming - previous) > SAFE_CTRL_MAX_STEP) {
        return previous + SAFE_CTRL_MAX_STEP;
    }
    if (previous > incoming && (unsigned char)(previous - incoming) > SAFE_CTRL_MAX_STEP) {
        return previous - SAFE_CTRL_MAX_STEP;
    }
    return incoming;
}

unsigned char velocity_tier_to_level(unsigned char tier) {
    switch (tier & 0x03) {
        case 0: return 5;
        case 1: return 7;
        case 2: return 9;
        default: return FAST_PULSE_MAX_LEVEL;
    }
}

unsigned char ctrl_to_sustain(unsigned char v) {
    return (v * 15) / 127;
}

unsigned int ctrl_to_glide_step(unsigned char v) {
    if (v < 4) {
        return 0;
    }
    if (v >= 120) return 4;
    if (v >= 112) return 6;
    if (v >= 96)  return 9;
    if (v >= 80)  return 13;
    if (v >= 64)  return 18;
    if (v >= 48)  return 26;
    if (v >= 32)  return 38;
    if (v >= 16)  return 54;
    return 72;
}

unsigned char smooth_u8(unsigned char current, unsigned char target, unsigned char step) {
    if (current < target) {
        current += step;
        if (current > target) {
            current = target;
        }
    } else if (current > target) {
        if (current > step) {
            current -= step;
        } else {
            current = 0;
        }
        if (current < target) {
            current = target;
        }
    }

    return current;
}

unsigned char smooth_ctrl_with_deadzone(unsigned char current, unsigned char target, unsigned char deadzone, unsigned char step) {
    if (target <= deadzone) {
        return 0;
    }

    return smooth_u8(current, target, step);
}

unsigned char nibble_to_ctrl(unsigned char v) {
    v &= 0x0F;
    return (unsigned char)(((unsigned int)v * 127U) / 15U);
}

unsigned char fast_lfo_nibble_to_ctrl(unsigned char v) {
    v &= 0x0F;
    switch (v) {
        case 0: return 0;
        case 1: return 0;
        case 2: return 0;
        case 3: return 16;
        case 4: return 17;
        case 5: return 18;
        case 6: return 19;
        case 7: return 21;
        case 8: return 24;
        case 9: return 28;
        case 10: return 34;
        case 11: return 42;
        case 12: return 54;
        case 13: return 68;
        case 14: return 84;
        default: return 100;
    }
}

signed char vibrato_step(unsigned char depth_ctrl, unsigned char rate_ctrl, unsigned char* counter, unsigned char* phase) {
    unsigned char depth = ctrl_to_vibrato_depth(depth_ctrl);
    unsigned char phase_step = ctrl_to_vibrato_phase_step(rate_ctrl);
    signed int scaled;
    signed char shape;

    if (depth == 0 || phase_step == 0) {
        *counter = 0;
        *phase = 0;
        return 0;
    }

    *counter = 0;
    *phase = (unsigned char)(*phase + phase_step);
    shape = vibrato_shape[(*phase >> 3) & 0x1F];

    if (depth_ctrl < 19) {
        if (shape >= 16 && ((*phase & 0x60) == 0x20)) return 1;
        if (shape <= -16 && ((*phase & 0x60) == 0x60)) return -1;
        return 0;
    }
    if (depth_ctrl < 24) {
        if (shape >= 16) return 1;
        if (shape <= -16) return -1;
        return 0;
    }
    if (depth_ctrl < 32) {
        if (shape >= 13) return 1;
        if (shape <= -13) return -1;
        return 0;
    }

    scaled = ((signed int)shape * (signed int)depth);

    if (scaled >= 0) {
        scaled += 8;
    } else {
        scaled -= 8;
    }

    return (signed char)(scaled / 16);
}

unsigned char ctrl_to_vibrato_depth(unsigned char v) {
    if (v < 16) {
        return 0;
    }
    return 1 + (unsigned char)(((unsigned int)(v - 16) * 18U) / 111U);
}

unsigned char ctrl_to_vibrato_phase_step(unsigned char v) {
    if (v < 8)   return 0;
    if (v < 18)  return 2;
    if (v < 30)  return 3;
    if (v < 44)  return 4;
    if (v < 58)  return 5;
    if (v < 72)  return 6;
    if (v < 86)  return 10;
    if (v < 100) return 22;
    if (v < 112) return 40;
    if (v < 120) return 72;
    return 120;
}

unsigned int glide_timer_towards(unsigned int current, unsigned int target, unsigned int step) {
    if (step == 0) {
        return target;
    }

    if (current < target) {
        current += step;
        if (current > target) {
            current = target;
        }
    } else if (current > target) {
        if (current > step) {
            current -= step;
        } else {
            current = 0;
        }
        if (current < target) {
            current = target;
        }
    }

    return current;
}

unsigned char velocity_to_15(unsigned char vel) {
    unsigned char out = vel >> 3;
    if (out == 0) out = 1;
    if (out > 15) out = 15;
    return out;
}

unsigned char pulse_velocity_to_15(unsigned char vel) {
    unsigned char out = 13 + (unsigned char)(((unsigned int)vel * 2U) / 127U);
    if (out > 15) {
        out = 15;
    }
    return out;
}

unsigned char decode_lfo_target(unsigned char v) {
    if (v == LFO_TARGET_DUTY) {
        return LFO_TARGET_DUTY;
    }
    if (v == LFO_TARGET_AMP) {
        return LFO_TARGET_AMP;
    }
    return LFO_TARGET_PITCH;
}

unsigned char modulated_duty(unsigned char base_duty, signed char lfo_offset) {
    int duty = base_duty;

    if (lfo_offset >= 14) {
        duty += 2;
    } else if (lfo_offset >= 6) {
        duty += 1;
    } else if (lfo_offset <= -14) {
        duty -= 2;
    } else if (lfo_offset <= -6) {
        duty -= 1;
    }

    if (duty < 0) {
        duty = 0;
    }
    if (duty > 3) {
        duty = 3;
    }
    return (unsigned char)duty;
}

unsigned char modulated_level(unsigned char base_level, signed char lfo_offset) {
    int level = base_level + (lfo_offset / 2);

    if (level < 0) {
        level = 0;
    }
    if (level > 15) {
        level = 15;
    }
    return (unsigned char)level;
}

unsigned char scaled_volume(unsigned char env_volume, unsigned char velocity) {
    unsigned int v = (unsigned int)env_volume * (unsigned int)velocity_to_15(velocity);
    v = (v + 7) / 15;
    if (v > 15) {
        v = 15;
    }
    return (unsigned char)v;
}

unsigned char pulse_scaled_volume(unsigned char env_volume, unsigned char velocity) {
    unsigned int v = (unsigned int)env_volume * (unsigned int)pulse_velocity_to_15(velocity);
    v = (v + 7) / 15;
    if (v > 15) {
        v = 15;
    }
    return (unsigned char)v;
}

unsigned char pulse_mix_volume(unsigned char v) {
    v = (unsigned char)((v * 13 + 7) / 15);
    if (v > 15) {
        v = 15;
    }
    return v;
}

unsigned char noise_mix_volume(unsigned char v) {
    v = (unsigned char)((v * 10U + 7U) / 15U);
    return v;
}

unsigned int offset_timer(unsigned int base, signed char offset) {
    if (offset >= 0) {
        return base + (unsigned char)offset;
    }

    offset = -offset;
    if (base > (unsigned char)offset) {
        return base - (unsigned char)offset;
    }

    return 0;
}

void pulse_voice_trigger(PulseVoice* voice, unsigned char note, unsigned char velocity, unsigned char duty, unsigned char trigger, unsigned char instant) {
    voice->note = note;
    voice->velocity = velocity;
    voice->duty = duty;
    voice->gate = 1;
    voice->last_trigger = trigger;
    voice->env_phase = ENV_ATTACK;
    voice->env_counter = 0;
    voice->env_volume = 0;
    voice->last_lo = 0xFF;
    voice->last_hi = 0xFF;
    voice->target_t = pulse_timer_from_note(note);
    if (instant || voice->current_t == 0) {
        voice->current_t = voice->target_t;
    }
}

void pulse_voice_assign(PulseVoice* voice, unsigned char desired_gate, unsigned char note, unsigned char velocity, unsigned char duty, unsigned char trigger, unsigned char instant) {
    if (desired_gate) {
        if (!voice->gate || voice->note != note || voice->velocity != velocity || voice->last_trigger != trigger) {
            pulse_voice_trigger(voice, note, velocity, duty, trigger, instant);
        } else {
            voice->target_t = pulse_timer_from_note(note);
        }
    } else if (voice->gate) {
        voice->gate = 0;
        if (voice->env_phase != ENV_OFF) {
            voice->env_phase = ENV_RELEASE;
            voice->env_counter = 0;
        }
    } else {
        voice->duty = duty;
    }
}

void pulse_voice_step_env(PulseVoice* voice, unsigned char attack_rate, unsigned char decay_rate, unsigned char release_rate, unsigned char sustain_level) {
    switch (voice->env_phase) {
        case ENV_OFF:
            break;
        case ENV_ATTACK:
            if (attack_rate == 0) {
                voice->env_volume = 15;
                voice->env_phase = ENV_DECAY;
                voice->env_counter = 0;
                break;
            }
            voice->env_counter++;
            if (voice->env_counter >= attack_rate) {
                voice->env_counter = 0;
                if (voice->env_volume < 15) {
                    voice->env_volume++;
                } else {
                    voice->env_phase = ENV_DECAY;
                }
            }
            break;
        case ENV_DECAY:
            if (decay_rate == 0) {
                voice->env_volume = sustain_level;
                voice->env_phase = ENV_SUSTAIN;
                voice->env_counter = 0;
                break;
            }
            voice->env_counter++;
            if (voice->env_counter >= decay_rate) {
                voice->env_counter = 0;
                if (voice->env_volume > sustain_level) {
                    voice->env_volume--;
                } else {
                    voice->env_phase = ENV_SUSTAIN;
                }
            }
            break;
        case ENV_SUSTAIN:
            if (!voice->gate) {
                voice->env_phase = ENV_RELEASE;
                voice->env_counter = 0;
            }
            break;
        case ENV_RELEASE:
            if (release_rate == 0) {
                voice->env_volume = 0;
                voice->env_phase = ENV_OFF;
                voice->env_counter = 0;
                break;
            }
            voice->env_counter++;
            {
                unsigned char effective_release_rate = effective_release_rate_for_volume(release_rate, voice->env_volume);
                if (voice->env_counter >= effective_release_rate) {
                    voice->env_counter = 0;
                    if (voice->env_volume > 0) {
                        voice->env_volume--;
                    } else {
                        voice->env_phase = ENV_OFF;
                    }
                }
            }
            break;
    }
}

unsigned char pulse_render_level_from_env(PulseVoice* voice, unsigned char attack_rate, unsigned char release_rate) {
    (void)attack_rate;
    (void)release_rate;
    return pulse_mix_volume(pulse_scaled_volume(voice->env_volume, voice->velocity));
}

void triangle_voice_assign(TriangleVoice* voice, unsigned char desired_gate, unsigned char note, unsigned char trigger, unsigned char release_rate, unsigned char instant) {
    if (desired_gate) {
        unsigned char new_trigger = (!voice->gate) || (voice->note != note) || (voice->last_trigger != trigger);
        voice->note = note;
        voice->gate = 1;
        voice->last_trigger = trigger;
        voice->release_counter = 0;
        voice->target_t = triangle_timer_from_note(note);
        if (new_trigger && BUS_TRI_PUNCH >= 64) {
            voice->punch_amount = 2;
            voice->punch_counter = 3;
        } else if (new_trigger) {
            voice->punch_amount = 0;
            voice->punch_counter = 0;
        }
        if (new_trigger && (instant || voice->current_t == 0)) {
            voice->current_t = voice->target_t;
        }
    } else if (voice->gate) {
        voice->gate = 0;
        voice->release_counter = release_rate;
        if (voice->release_counter == 0) {
            voice->current_t = 0;
        }
    }
}

void triangle_voice_step(TriangleVoice* voice, unsigned int glide_step) {
    if (voice->gate) {
        voice->target_t = triangle_timer_from_note(voice->note);
        voice->current_t = glide_timer_towards(voice->current_t, voice->target_t, glide_step);
    } else if (voice->release_counter > 0) {
        voice->release_counter--;
        if (voice->release_counter == 0) {
            voice->current_t = 0;
        }
    }

    if (voice->punch_counter > 0) {
        voice->punch_counter--;
    }
}

unsigned char triangle_voice_is_active(TriangleVoice* voice) {
    return voice->gate || (voice->release_counter > 0);
}

void noise_voice_trigger(NoiseVoice* voice, unsigned char note, unsigned char velocity, unsigned char timbre_ctrl, unsigned char mode_ctrl) {
    voice->note = note;
    voice->velocity = velocity;
    voice->gate = 1;
    voice->env_phase = ENV_ATTACK;
    voice->env_counter = 0;
    noise_voice_configure_drum(voice, note, timbre_ctrl, mode_ctrl);
    voice->hold_counter = voice->hold_frames;
    if (voice->drum_kind == DRUM_KICK || voice->drum_kind == DRUM_SNARE) {
        voice->env_volume = 4;
    } else if (voice->drum_kind == DRUM_CLHAT || voice->drum_kind == DRUM_OPHAT || voice->drum_kind == DRUM_CYMBAL) {
        voice->env_phase = ENV_DECAY;
        voice->env_volume = 15;
    } else {
        voice->env_volume = 2;
    }
    voice->mode = voice->transient_mode;
    voice->period = voice->transient_period;
}

void noise_voice_assign(NoiseVoice* voice, unsigned char desired_gate, unsigned char note, unsigned char velocity, unsigned char trigger, unsigned char timbre_ctrl, unsigned char mode_ctrl) {
    if (desired_gate) {
        if (voice->note != note || voice->velocity != velocity || voice->last_trigger != trigger) {
            noise_voice_trigger(voice, note, velocity, timbre_ctrl, mode_ctrl);
            voice->last_trigger = trigger;
        }
    } else if (voice->gate) {
        voice->gate = 0;
        if (voice->env_phase != ENV_OFF) {
            voice->env_phase = ENV_RELEASE;
            voice->env_counter = 0;
        }
    }
}

void noise_voice_step_tone(NoiseVoice* voice) {
    if (voice->env_phase == ENV_OFF) {
        return;
    }

    if (voice->gate && voice->hold_counter > 0) {
        voice->hold_counter--;
        if (voice->hold_counter == 0) {
            voice->gate = 0;
        }
    }

    if (voice->transient_frames > 0) {
        voice->transient_frames--;
        voice->mode = voice->transient_mode;
        voice->period = voice->transient_period;
    } else if (voice->tail_frames > 0) {
        voice->tail_frames--;
        voice->mode = voice->tail_mode;
        voice->period = voice->tail_period;
    } else {
        voice->mode = voice->body_mode;
        voice->period = voice->body_period;
    }
}

void noise_voice_step_env(NoiseVoice* voice, unsigned char attack_rate, unsigned char decay_rate, unsigned char release_rate, unsigned char sustain_level) {
    decay_rate = noise_shaped_rate(decay_rate, voice->drum_kind, 0);
    release_rate = noise_shaped_rate(release_rate, voice->drum_kind, 1);
    decay_rate += noise_note_length_add(voice->note, voice->drum_kind, 0);
    release_rate += noise_note_length_add(voice->note, voice->drum_kind, 1);

    switch (voice->drum_kind) {
        case DRUM_KICK:
        case DRUM_SNARE:
        case DRUM_CLHAT:
            sustain_level = 0;
            break;
        case DRUM_TOM:
            if (sustain_level > 3) {
                sustain_level = 3;
            }
            break;
        case DRUM_OPHAT:
            if (sustain_level > 1) {
                sustain_level = 1;
            }
            break;
        case DRUM_CYMBAL:
            if (sustain_level > 1) {
                sustain_level = 1;
            }
            break;
    }

    switch (voice->env_phase) {
        case ENV_OFF:
            break;
        case ENV_ATTACK:
            if (attack_rate == 0) {
                voice->env_volume = 15;
                voice->env_phase = ENV_DECAY;
                voice->env_counter = 0;
                break;
            }
            voice->env_counter++;
            if (voice->env_counter >= attack_rate) {
                voice->env_counter = 0;
                if (voice->env_volume < 15) {
                    voice->env_volume++;
                } else {
                    voice->env_phase = ENV_DECAY;
                }
            }
            break;
        case ENV_DECAY:
            if (decay_rate == 0) {
                voice->env_volume = sustain_level;
                voice->env_phase = ENV_SUSTAIN;
                voice->env_counter = 0;
                break;
            }
            voice->env_counter++;
            if (voice->env_counter >= decay_rate) {
                voice->env_counter = 0;
                if (voice->env_volume > sustain_level) {
                    voice->env_volume--;
                } else {
                    voice->env_phase = ENV_SUSTAIN;
                }
            }
            break;
        case ENV_SUSTAIN:
            if (!voice->gate) {
                voice->env_phase = ENV_RELEASE;
                voice->env_counter = 0;
            }
            break;
        case ENV_RELEASE:
            if (release_rate == 0) {
                voice->env_volume = 0;
                voice->env_phase = ENV_OFF;
                voice->env_counter = 0;
                break;
            }
            voice->env_counter++;
            if (voice->env_counter >= release_rate) {
                voice->env_counter = 0;
                if (voice->env_volume > 0) {
                    voice->env_volume--;
                } else {
                    voice->env_phase = ENV_OFF;
                }
            }
            break;
    }
}

void main(void) {
    PulseVoice p1 = {60, 100, 0, 0xFF, 2, ENV_OFF, 0, 0, 0xFF, 0xFF, 0, 0};
    PulseVoice p2 = {64, 100, 0, 0xFF, 2, ENV_OFF, 0, 0, 0xFF, 0xFF, 0, 0};
    TriangleVoice tri = {48, 0, 0xFF, 0, 0, 0, 0xFF, 0xFF, 0, 0};
    NoiseVoice noi = {36, 100, 0, 0xFF, DRUM_KICK, ENV_OFF, 0, 0, 0x0F, 0xFF, 0x00, 0, 0x0F, 0x00, 0, 0x0F, 0x00, 0, 0, 0x0F, 0x00};

    unsigned char p1_attack_rate;
    unsigned char p1_decay_rate;
    unsigned char p1_release_rate;
    unsigned char p1_sustain_level;
    unsigned char p2_attack_rate;
    unsigned char p2_decay_rate;
    unsigned char p2_release_rate;
    unsigned char p2_sustain_level;
    unsigned char tri_release_rate;
    unsigned char noi_attack_rate;
    unsigned char noi_decay_rate;
    unsigned char noi_release_rate;
    unsigned char noi_sustain_level;
    unsigned char p1_glide_ctrl = 0;
    unsigned char p2_glide_ctrl = 0;
    unsigned char tri_glide_ctrl = 0;
    unsigned char p1_vib_depth_ctrl = 0;
    unsigned char p1_vib_rate_ctrl = 0;
    unsigned char p2_vib_depth_ctrl = 0;
    unsigned char p2_vib_rate_ctrl = 0;
    unsigned char tri_vib_depth_ctrl = 0;
    unsigned char tri_vib_rate_ctrl = 0;
    unsigned char noi_timbre_ctrl = 0;
    unsigned int p1_glide_step;
    unsigned int p2_glide_step;
    unsigned int tri_glide_step;
    unsigned char p1_vibrato_counter = 0;
    unsigned char p1_vibrato_phase = 0;
    unsigned char p2_vibrato_counter = 0;
    unsigned char p2_vibrato_phase = 0;
    unsigned char tri_vibrato_counter = 0;
    unsigned char tri_vibrato_phase = 0;
    signed char p1_vibrato_offset = 0;
    signed char p2_vibrato_offset = 0;
    signed char tri_vibrato_offset = 0;
    signed char tri_pitch_offset = 0;
    unsigned char p1_duty;
    unsigned char p2_duty;
    unsigned char p1_render_duty;
    unsigned char p2_render_duty;
    unsigned char p1_lfo_target;
    unsigned char p2_lfo_target;
    unsigned char tri_lfo_target;
    unsigned char p1_level;
    unsigned char p2_level;
    unsigned char p1_render_level;
    unsigned char p2_render_level;
    unsigned char noi_level;
    unsigned char lo;
    unsigned char hi;
    unsigned int render_t;
    unsigned char dmc_gate = 0;
    unsigned char dmc_note = 0;
    unsigned char dmc_velocity = 0;
    unsigned char dmc_last_trigger = 0xFF;
    unsigned char controller_rx_poll_counter = 0;
    const DmcSample* dmc_sample;

    APU_CTRL = APU_ENABLE_STD;
    DMC_RAW = 0x50;
    PULSE1_VOL = 0x30;
    PULSE1_SWEEP = 0x00;
    PULSE2_VOL = 0x30;
    PULSE2_SWEEP = 0x00;
    TRI_LINEAR = 0x00;
    NOISE_VOL = 0x30;
    NOISE_LO = 0x0F;
    init_bus_defaults();
    // ppu_boot_init supprimé - perturbe l'initialisation

// Supprimé: boot_diag_menu() inutile

#if CONTROLLER_RX_FAST_P2
    controller_rx_fast_p2_loop();
#endif

#if DIAG_AUTOTONE
    {
        unsigned char i;
        for (i = 0; i < BUS_PACKET_SIZE; ++i) {
            BUS_BASE[i] = 0;
        }
    }
    BUS_P2_NOTE = 64;
    BUS_P2_VEL = 110;
    BUS_P2_GATE = 1;
    BUS_P2_DUTY = 2;
    BUS_P2_ATTACK = 0;
    BUS_P2_DECAY = 0;
    BUS_P2_SUSTAIN = 127;
    BUS_P2_RELEASE = 0;
    BUS_P2_TRIG = 1;
#endif

    while (1) {
        wait_vblank();

#if DIAG_BIT_TONE
        BUS_P1_GATE = 0;
        BUS_TRI_GATE = 0;
        BUS_NOI_GATE = 0;
        BUS_P2_NOTE = controller_rx_read_bit() ? 72 : 48;
        BUS_P2_VEL = 110;
        BUS_P2_GATE = 1;
        BUS_P2_DUTY = 2;
        BUS_P2_ATTACK = 0;
        BUS_P2_DECAY = 0;
        BUS_P2_SUSTAIN = 127;
        BUS_P2_RELEASE = 0;
        BUS_P2_TRIG = BUS_P2_NOTE;
#elif CONTROLLER_RX_ENABLED
        if (controller_rx_poll_counter == 0) {
// Utiliser toujours le mode compact pour lire les paquets parallèles DN0/DN3
            controller_rx_apply_compact_packet();
#if CONTROLLER_RX_POLL_DIV > 1
            controller_rx_poll_counter = CONTROLLER_RX_POLL_DIV - 1;
#endif
        } else {
            --controller_rx_poll_counter;
        }

#if DIAG_PORT_RAW
        BUS_P1_GATE = 0;
        BUS_TRI_GATE = 0;
        BUS_NOI_GATE = 0;
        BUS_P2_NOTE = 36 + (BUS_MAGIC_A & 0x3F);
        BUS_P2_VEL = 110;
        BUS_P2_GATE = 1;
        BUS_P2_DUTY = 2;
        BUS_P2_ATTACK = 0;
        BUS_P2_DECAY = 0;
        BUS_P2_SUSTAIN = 127;
        BUS_P2_RELEASE = 0;
        BUS_P2_TRIG = BUS_MAGIC_B;
#else
        if (BUS_MAGIC_A != 0xA8 || BUS_MAGIC_B != 0x58) {
            BUS_P1_GATE = 0;
            BUS_P2_GATE = 0;
            BUS_TRI_GATE = 0;
            BUS_NOI_GATE = 0;
        }
#endif
#endif

        p1_attack_rate = ctrl_to_attack_rate(BUS_P1_ATTACK);
        p1_decay_rate = ctrl_to_decay_rate(BUS_P1_DECAY);
        p1_release_rate = ctrl_to_release_rate(BUS_P1_RELEASE);
        p1_sustain_level = ctrl_to_sustain(BUS_P1_SUSTAIN);

        p2_attack_rate = ctrl_to_attack_rate(BUS_P2_ATTACK);
        p2_decay_rate = ctrl_to_decay_rate(BUS_P2_DECAY);
        p2_release_rate = ctrl_to_release_rate(BUS_P2_RELEASE);
        p2_sustain_level = ctrl_to_sustain(BUS_P2_SUSTAIN);

        tri_release_rate = ctrl_to_release_rate(BUS_TRI_RELEASE);

        noi_attack_rate = ctrl_to_attack_rate(BUS_NOI_ATTACK);
        noi_decay_rate = ctrl_to_decay_rate(BUS_NOI_DECAY);
        noi_release_rate = ctrl_to_release_rate(BUS_NOI_RELEASE);
        noi_sustain_level = ctrl_to_sustain(BUS_NOI_SUSTAIN);
        p1_glide_ctrl = smooth_u8(p1_glide_ctrl, BUS_P1_GLIDE, 4);
        p2_glide_ctrl = smooth_u8(p2_glide_ctrl, BUS_P2_GLIDE, 4);
        tri_glide_ctrl = smooth_u8(tri_glide_ctrl, BUS_TRI_GLIDE, 4);
        p1_vib_depth_ctrl = smooth_ctrl_with_deadzone(p1_vib_depth_ctrl, BUS_P1_VIB_DEPTH, 15, 2);
        if (BUS_P1_VIB_RATE < 7) p1_vib_rate_ctrl = 0; else p1_vib_rate_ctrl = BUS_P1_VIB_RATE;
        p2_vib_depth_ctrl = smooth_ctrl_with_deadzone(p2_vib_depth_ctrl, BUS_P2_VIB_DEPTH, 15, 2);
        if (BUS_P2_VIB_RATE < 7) p2_vib_rate_ctrl = 0; else p2_vib_rate_ctrl = BUS_P2_VIB_RATE;
        tri_vib_depth_ctrl = smooth_ctrl_with_deadzone(tri_vib_depth_ctrl, BUS_TRI_VIB_DEPTH, 15, 2);
        if (BUS_TRI_VIB_RATE < 7) tri_vib_rate_ctrl = 0; else tri_vib_rate_ctrl = BUS_TRI_VIB_RATE;
        noi_timbre_ctrl = smooth_u8(noi_timbre_ctrl, BUS_NOI_TIMBRE, 4);

        p1_glide_step = ctrl_to_glide_step(p1_glide_ctrl);
        p2_glide_step = ctrl_to_glide_step(p2_glide_ctrl);
        tri_glide_step = ctrl_to_glide_step(tri_glide_ctrl);
        p1_duty = BUS_P1_DUTY & 0x03;
        p2_duty = BUS_P2_DUTY & 0x03;
        p1_vibrato_offset = vibrato_step(p1_vib_depth_ctrl, p1_vib_rate_ctrl, &p1_vibrato_counter, &p1_vibrato_phase);
        p2_vibrato_offset = vibrato_step(p2_vib_depth_ctrl, p2_vib_rate_ctrl, &p2_vibrato_counter, &p2_vibrato_phase);
        tri_vibrato_offset = vibrato_step(tri_vib_depth_ctrl, tri_vib_rate_ctrl, &tri_vibrato_counter, &tri_vibrato_phase) >> 1;
        p1_lfo_target = decode_lfo_target(BUS_P1_LFO_TARGET);
        p2_lfo_target = decode_lfo_target(BUS_P2_LFO_TARGET);
        tri_lfo_target = decode_lfo_target(BUS_TRI_LFO_TARGET);

        pulse_voice_assign(&p1, BUS_P1_GATE ? 1 : 0, BUS_P1_NOTE, BUS_P1_VEL, p1_duty, BUS_P1_TRIG, BUS_P1_GLIDE == 0);
        pulse_voice_assign(&p2, BUS_P2_GATE ? 1 : 0, BUS_P2_NOTE, BUS_P2_VEL, p2_duty, BUS_P2_TRIG, BUS_P2_GLIDE == 0);
        triangle_voice_assign(&tri, BUS_TRI_GATE ? 1 : 0, BUS_TRI_NOTE, BUS_TRI_TRIG, tri_release_rate, BUS_TRI_GLIDE == 0);
        dmc_sample = dmc_sample_from_note(BUS_NOI_NOTE);
        if (BUS_NOI_GATE && dmc_sample != 0) {
            if (!dmc_gate || dmc_note != BUS_NOI_NOTE || dmc_velocity != BUS_NOI_VEL || dmc_last_trigger != BUS_NOI_TRIG) {
                dmc_trigger(dmc_sample);
                dmc_note = BUS_NOI_NOTE;
                dmc_velocity = BUS_NOI_VEL;
                dmc_last_trigger = BUS_NOI_TRIG;
            }
            dmc_gate = 1;
            noise_voice_silence(&noi);
        } else {
            dmc_gate = 0;
            noise_voice_assign(&noi, BUS_NOI_GATE ? 1 : 0, BUS_NOI_NOTE, BUS_NOI_VEL, BUS_NOI_TRIG, noi_timbre_ctrl, BUS_NOI_MODE);
        }

        pulse_voice_step_env(&p1, p1_attack_rate, p1_decay_rate, p1_release_rate, p1_sustain_level);
        pulse_voice_step_env(&p2, p2_attack_rate, p2_decay_rate, p2_release_rate, p2_sustain_level);
        noise_voice_step_env(&noi, noi_attack_rate, noi_decay_rate, noi_release_rate, noi_sustain_level);
        noise_voice_step_tone(&noi);
        triangle_voice_step(&tri, tri_glide_step);
        if (tri_lfo_target == LFO_TARGET_PITCH || tri_lfo_target == LFO_TARGET_DUTY || tri_lfo_target == LFO_TARGET_AMP) {
            tri_pitch_offset = tri_vibrato_offset;
        } else {
            tri_pitch_offset = tri_vibrato_offset;
        }
        if (tri.punch_counter > 0 && tri.punch_amount > 0) {
            tri_pitch_offset += (signed char)(tri.punch_amount * tri.punch_counter);
        }

        if (p1.env_phase != ENV_OFF) {
            p1.target_t = pulse_timer_from_note(p1.note);
            p1.current_t = glide_timer_towards(p1.current_t, p1.target_t, p1_glide_step);
            p1_level = pulse_render_level_from_env(&p1, p1_attack_rate, p1_release_rate);
            p1_render_duty = p1.duty;
            p1_render_level = p1_level;
            if (p1_lfo_target == LFO_TARGET_DUTY) {
                p1_render_duty = modulated_duty(p1.duty, p1_vibrato_offset);
                render_t = p1.current_t;
            } else if (p1_lfo_target == LFO_TARGET_AMP) {
                p1_render_level = modulated_level(p1_level, p1_vibrato_offset);
                render_t = p1.current_t;
            } else {
                render_t = offset_timer(p1.current_t, p1_vibrato_offset);
            }
            lo = render_t & 0xFF;
            hi = ((render_t >> 8) & 0x07) | 0xF8;
            PULSE1_VOL = (p1_render_duty << 6) | 0x30 | p1_render_level;
            PULSE1_SWEEP = 0x00;
            if (lo != p1.last_lo) {
                PULSE1_LO = lo;
                p1.last_lo = lo;
            }
            if (hi != p1.last_hi) {
                PULSE1_HI = hi;
                p1.last_hi = hi;
            }
        } else {
            PULSE1_VOL = 0x30;
            p1.last_lo = 0xFF;
            p1.last_hi = 0xFF;
        }

        if (p2.env_phase != ENV_OFF) {
            p2.target_t = pulse_timer_from_note(p2.note);
            p2.current_t = glide_timer_towards(p2.current_t, p2.target_t, p2_glide_step);
            p2_level = pulse_render_level_from_env(&p2, p2_attack_rate, p2_release_rate);
            p2_render_duty = p2.duty;
            p2_render_level = p2_level;
            if (p2_lfo_target == LFO_TARGET_DUTY) {
                p2_render_duty = modulated_duty(p2.duty, p2_vibrato_offset);
                render_t = p2.current_t;
            } else if (p2_lfo_target == LFO_TARGET_AMP) {
                p2_render_level = modulated_level(p2_level, p2_vibrato_offset);
                render_t = p2.current_t;
            } else {
                render_t = offset_timer(p2.current_t, p2_vibrato_offset);
            }
            lo = render_t & 0xFF;
            hi = ((render_t >> 8) & 0x07) | 0xF8;
            PULSE2_VOL = (p2_render_duty << 6) | 0x30 | p2_render_level;
            PULSE2_SWEEP = 0x00;
            if (lo != p2.last_lo) {
                PULSE2_LO = lo;
                p2.last_lo = lo;
            }
            if (hi != p2.last_hi) {
                PULSE2_HI = hi;
                p2.last_hi = hi;
            }
        } else {
            PULSE2_VOL = 0x30;
            p2.last_lo = 0xFF;
            p2.last_hi = 0xFF;
        }

        if (triangle_voice_is_active(&tri)) {
            render_t = offset_timer(tri.current_t, tri_pitch_offset);
            lo = render_t & 0xFF;
            hi = (render_t >> 8) & 0x07;
            TRI_LINEAR = 0x80 | 0x7F;

            if (lo != tri.last_lo) {
                TRI_LO = lo;
                tri.last_lo = lo;
            }
            if (hi != tri.last_hi) {
                TRI_HI = hi;
                tri.last_hi = hi;
            }
        } else {
            TRI_LINEAR = 0x00;
            tri.last_lo = 0xFF;
            tri.last_hi = 0xFF;
        }

        if (noi.env_phase != ENV_OFF) {
            noi_level = scaled_volume(noi.env_volume, noi.velocity);
            noi_level = noise_mix_volume(noi_level);
            NOISE_VOL = 0x30 | noi_level;
            if (noi.period != noi.last_period) {
                NOISE_LO = noi.mode | noi.period;
                noi.last_period = noi.period;
            } else {
                NOISE_LO = noi.mode | noi.period;
            }
            NOISE_HI = 0xF8;
        } else {
            NOISE_VOL = 0x30;
            noi.last_period = 0xFF;
        }
    }
}
