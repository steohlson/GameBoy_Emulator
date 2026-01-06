#ifndef DEBUG_H
#define DEBUG_H

#include "../emulator/gb.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define DEBUG_MODE //comment out to stop debug logging

typedef struct {
    uint16_t PC;
    uint16_t SP;
    uint16_t AF;
    uint16_t BC;
    uint16_t DE;
    uint16_t HL;
    uint32_t MEM_PC;
    uint8_t PPU_MODE;
    float NS_TAKEN;
    float NS_OVER;
    float NS_CPU;
    float NS_PPU;

} DebugOutput;

extern DebugOutput *debug_output;

void debugLogInit();
void debugLog();
void debugCleanup();



#endif // !DEBUG_H