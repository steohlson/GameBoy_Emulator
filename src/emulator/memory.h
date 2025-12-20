#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <string.h>

#define ROM 0x0000
#define SWITCHABLE_ROM 0x4000
#define V_RAM 0x8000
#define SWITCHABLE_RAM 0xA000
#define INTERNAL_RAM 0xC000
#define SPRITE_ATTRIBUTES 0xFE00
#define I_O 0xFF00
#define HIGH_RAM 0xFF80
#define IE 0xFFFF //Interrupt Enable Register
#define IF 0xFF0F //Interrupt Flag Register


void memory_init();
uint8_t memory_get(uint16_t address);
void memory_set(uint16_t address, uint8_t value);

#endif // !MEMORY_H
