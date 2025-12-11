#include "memory.h"



uint8_t memory[65536];


void memory_init() {
    for(int i=0; i < sizeof(memory) / sizeof(memory[0]); i++)     { memory[i] = 0; }
}


uint8_t memory_get(uint16_t address) {
    return memory[address];
}

void memory_set(uint16_t address, uint8_t value) {
    memory[address] = value;
}
