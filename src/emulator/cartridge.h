#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include "gb.h"
#include "../platform/platform.h"

extern Rom cartridge_rom;

void cartridge_load();
uint8_t cartridge_get(uint16_t address);


#endif // !CARTRIDGE_H