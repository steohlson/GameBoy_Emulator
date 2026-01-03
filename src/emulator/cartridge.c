#include "cartridge.h"

static Rom rom = {0};

void cartridge_load() {
    rom = platform_file_load();
}

uint8_t cartridge_get(uint16_t address) {
    if(address >= rom.size) {
        return 0xFF;
    }
    return rom.data[address];
}