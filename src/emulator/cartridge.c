#include "cartridge.h"

Rom cartridge_rom = {0};

void cartridge_load() {
    platform_file_load(&cartridge_rom);
}

uint8_t cartridge_get(uint16_t address) {
    if(address >= cartridge_rom.size) {
        return 0xFF;
    }
    return cartridge_rom.data[address];
}