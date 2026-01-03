#include "memory.h"


#include "bootrom.h"

uint8_t memory[65536];
bool boot_rom_enabled = true;


void memory_init() {
    for(int i=0; i < sizeof(memory) / sizeof(memory[0]); i++)     { memory[i] = 0; }
    boot_rom_enabled = true;
}


uint8_t memory_get(uint16_t address) {
    if(address < 0x8000) {
        if(address < 0x0100 && boot_rom_enabled) {
            return dmg_boot_bin[address];
        }
        return cartridge_get(address);
    }
    return memory[address];
}

void memory_set(uint16_t address, uint8_t value) {
    if(address == DIV) {
        memory[address] = 0;
        return;
    }
    if(address == 0xFF50) {
        boot_rom_enabled = false;
        return;
    }
    memory[address] = value;
}

void memory_inc(uint16_t address) {
    memory[address] += 1;
}
