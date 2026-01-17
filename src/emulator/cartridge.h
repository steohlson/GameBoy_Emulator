#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include "gb.h"
#include "../platform/platform.h"

//Now defined in memory.c

extern Rom cartridge_rom;
extern uint16_t cartridge_offset;

#define ROM_ONLY                0x00
#define MBC1                    0x01
#define MBC1_RAM                0x02
#define MBC1_RAM_BATT           0x03
#define MBC2                    0x05
#define MBC2_BATT               0x06
#define ROM_RAM                 0x08 //no licensed rom uses this
#define ROM_RAM_BATT            0x09 //no licensed rom uses this
#define MMM01                   0x0B
#define MMM01_RAM               0x0B
#define MMM01_RAM_BATT          0x0B
#define MBC3_TIMER_BATT         0x0F
#define MBC3_TIMER_RAM_BATT     0x10
#define MBC3                    0x11
#define MBC3_RAM                0x12
#define MBC3_RAM_BATT           0x13
#define MBC5                    0x19
#define MBC5_RAM                0x1A
#define MBC5_RAM_BATT           0x1B
#define MBC5_RUMBLE             0x1C
#define MBC5_RUMBLE_RAM         0x1D
#define MBC5_RUMBLE_RAM_BATT    0x1E
#define MBC6                    0x20
#define MBC7_SENSOR_RUMBLE_RAM_BATT 0x22
#define POCKET_CAMERA           0xFE
#define BANDAI_TAMA5            0xFD
#define HuC3                    0xFE
#define HuC1_RAM_BATT           0xFF

void cartridge_load();
uint8_t cartridge_get(uint16_t address);
void cartridge_set(uint16_t address, uint8_t value);


#endif // !CARTRIDGE_H