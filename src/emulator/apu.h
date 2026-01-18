#ifndef APU_H
#define APU_H

#include "memory.h"
#include "gb.h"

void apu_init();
void apu_update();



//NR5y : Global control registers

//Audio Master Control
#define NR52 0xFF26
//Sound Panning
#define NR51 0xFF25
//Master volume and VIN panning
#define NR50 0xFF24


//NR1y : Channel 1 - Pulse with period sweep

//Channel 1 sweep
#define NR10 0xFF10
//Channel 1 length timer and duty cycle
#define NR11 0xFF11
//Channel 1 volume and envelope
#define NR12 0xFF12
//Channel 1 period low [write only]
#define NR13 0xFF13
//Channel 1 period high and control
#define NR14 0xFF14


//NR2y : Channel 2 - Pulse

//Channel 2 length timer and duty cycle
#define NR21 0xFF16
//Channel 2 volume and envelope
#define NR22 0xFF17
//Channel 2 period low [write only]
#define NR23 0xFF18
//Channel 2 period high and control
#define NR24 0xFF19


//NR3y : Channel 3 - Wave output

//Channel 3 DAC enable
#define NR30 0xFF1A
//Channel 3 length timer [write only]
#define NR31 0xFF1B
//Channel 3 output level
#define NR32 0xFF1C
//Channel 3 perod low [write only]
#define NR33 0xFF1D
//Channel 3 period high and control
#define NR34 0xFF1E

//Wave pattern ram FF30-FF3F
#define WAVE_PATTERN_RAM 0xFF30


//NR4y : Channel 4 - Noise

//Channel 4 length timer
#define NR41 0xFF20
//Channel 4 volume and envelope
#define NR42 0xFF21
//Channel 4 frequency and randomness
#define NR43 0xFF22
//Channel 4 control
#define NR44 0xFF23

#endif // !APU_H
