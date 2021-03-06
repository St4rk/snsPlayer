/*
 * NES Audio Processing Unit
 * Written by St4rk
 * Only the two square wave channels are implemented
 */
 
#ifndef _APU_H_
#define _APU_H_

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>

#include "6502.h"

/**
 * timer period lookup table to noise 
 * this is the NTSC version
 */
static const unsigned int noise_lookup_tbl[] = {
	4, 8, 16, 32, 64, 96, 128, 160,
	202, 254, 380, 508, 762, 1016,
	2034, 4068
};

static const float duty_freq[] = {
	0.125f, 0.25f, 0.5f, 0.75f
};

static const unsigned char triangle_sequence[] = {
	0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8, 
	0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
};

static const unsigned int dmc_rate[] = {
	428, 380, 340, 320, 286, 254, 226, 214, 
	190, 160, 142, 128, 106,  84,  72,  54
};

#define INTERNAL_VOLUME     100
#define MAX_SAMPLES         512

#define SQUARE_WAVE_UNIT_1  0x0
#define SQUARE_WAVE_UNIT_2  0x1 

/* Square wave 1 */
#define PULSE1_DUTY_ENV     0x4000
#define PULSE1_SWEEP        0x4001
#define PULSE1_PERIOD_LOW   0x4002
#define PULSE1_LEN_PE_HIGH  0x4003
/* Square wave 2 */
#define PULSE2_DUTY_ENV     0x4004
#define PULSE2_SWEEP        0x4005
#define PULSE2_PERIOD_LOW   0x4006
#define PULSE2_LEN_PE_HIGH  0x4007
/* Triangle wave */
#define TRIANGLE_CNT_LOAD   0x4008
#define TRIANGLE_UNUSED     0x4009
#define TRIANGLE_TMR_LOW    0x400A
#define TRIANGLE_TMR_HIGH   0x400B
/* Noise wave */
#define NOISE_ENV           0x400C
#define NOISE_UNUSED        0x400D
#define NOISE_L_PERIOD      0x400E
#define NOISE_LEN_CNT       0x400F
/* DMC wave */
#define DMC_ILF             0x4010
#define DMC_LOAD_CNT        0x4011
#define DMC_SAMPLE_ADDR     0x4012
#define DMC_SAMPLE_LEN      0x4013

/* CPU Frequency */
#define NTSC_CPU_CLOCK      1789773
#define PAL_CPU_CLOK        1662607

/* Status */
#define STATUS_REGISTER     0x4015
/* Frame Counter */
#define FRAME_CNT_REG       0x4017

typedef struct sweep {
	unsigned char enable;
	unsigned char period;
	unsigned char neg_flag;
	unsigned char shift_cnt;
	unsigned char divider_cnt;
} sweep;

typedef struct evenlope {
	/* envelope decay loop */
	unsigned char loop_flag;
	/* envelope decay flag (disable/enable) */
	unsigned char c_flag;
	/* Volume and Envelope decay rate */
	unsigned char volume;
	/* 4 bit internal down counter */
	unsigned char down_cnt;
} envelope;

typedef struct square_wave {
	/* 11 Bit Timer used by square wave 1 and 2 */
	/* It's the wavelenght */
	unsigned short timer;
	/* duty value used by square wave 1 and 2 */
	//float duty;
	unsigned char duty;
	/* Length counter */
	unsigned char len_cnt;
	/* Envelope Unit */
	envelope env;
	/* Sweep Unit */
	sweep    swp;

	unsigned int out_freq;
} square_wave;

typedef struct triangle_wave {
	/* 11 Bit Timer used by triangle wave */
	/* It's the wavelenght */
	unsigned int timer;
	unsigned int tmr_cnt;

	/* Length counter */
	unsigned char len_cnt;

	/* Length Counter halt / linear 
	   counter control */
	unsigned char controlFlag;

	/* Linear Counter */
	unsigned char linear;
	unsigned char linear_cnt;
	unsigned char haltFlag;

	/* triangle sequencer */
	unsigned int triSequence;
	unsigned char seqValue;

	/* Output frequency */
	unsigned int out_freq;

	/* Triangle 32 step output */
	unsigned char step;
	unsigned char cnt;

	/* old value */
	short oldOutput;
} triangle_wave;

typedef struct noise_wave {
	/* noise timer */
	unsigned int timer;
	/* noise timer cnt */
	unsigned int tmr_cnt;
	/* mode flag */
	unsigned char mode;
	/* Length Coutner */
	unsigned int len_cnt;
	/* out frequency */
	unsigned int out_freq;
	/* lfsr */
	unsigned short lfsr;

	envelope env; 
} noise_wave;

typedef struct apu_dmc {
	unsigned char irq;
	unsigned char loop;
	unsigned char freq;

	unsigned char directLoad;

	unsigned short addr;
	unsigned short addrCnt;
	unsigned char size;
	unsigned char sizeCnt;

	unsigned char sample;
	unsigned char shift;

	unsigned char dacCnt;

	unsigned int out_freq;
} apu_dmc;

typedef struct apu_status {
	unsigned char dmc_flag;
	unsigned char noise_flag;
	unsigned char triangle_flag;
	unsigned char pulse_channel_2;
	unsigned char pulse_channel_1;
	/* 0 = 4 step mode, 1 = 5 step mode */
	unsigned char frame_cnt_mode;
	/* sequence used by the framecounter */
	unsigned int  frame_cnt_sec;
	unsigned char irq_flag;
	unsigned char dmc_interrupt;
} apu_status;


extern square_wave   squareList[2];
extern triangle_wave triangle;
extern noise_wave    noise;
extern apu_dmc       dmc;
extern apu_status    apu;
extern short         samples[MAX_SAMPLES];
/* Square Wave */
extern void square_envelope(unsigned char unit);
extern void square_sweep(unsigned char unit);
extern void square_timer(unsigned char unit);
extern void square_len_cnt(unsigned char unit);
extern void square_freq_output(unsigned char unit);
extern short square_sample(unsigned char unit);

/* Triangle Wave */
extern void triangle_freq_output();
extern void triangle_len_cnt();
extern void triangle_linear_cnt();
extern void triangle_step();
extern void triangle_timer();

extern void noise_len_cnt();
extern void noise_envelope();
extern void noise_out_freq();
extern void noise_timer();
extern void noise_lfsr();
extern void noise_out_freq();

extern void dmc_update_freq();
extern short dmc_samples();

/* SDL and APU Stuff */
/* APU Lenght Counter table */
extern unsigned char getLengthCnt(unsigned char len);
/* callback used to fill the audio buffer */
extern void fill_audio(void *data, Uint8* stream, int len); 
/* call used to initialize SDL Audio */
extern void open_audio();
/* call used to close SDL Audio */
extern void close_audio();
/* Mix the Square 1 and Square 2 output samples */
extern short mix_channel();
/* Apu Mix */
extern void apu_mix();
#endif