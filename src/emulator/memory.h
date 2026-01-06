#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "cartridge.h"


#define ROM 0x0000 //ROM Bank 0
#define SWITCHABLE_ROM 0x4000 //Switchable ROM Bank
#define V_RAM 0x8000 //Video RAM
#define SWITCHABLE_RAM 0xA000 //Switchable RAM
#define INTERNAL_RAM 0xC000 //Internal RAM
#define SPRITE_ATTRIBUTES 0xFE00 //Sprite Attribute Table
#define I_O 0xFF00 //I/O Ports
#define HIGH_RAM 0xFF80 //High RAM
#define OAM 0xFE00
//Serial
#define SB 0xFF01 //Serial Transfer Data Register
#define SC 0xFF02 //Serial Control Register

//Timer Registers
#define DIV 0xFF04 //Divider Register
#define TIMA 0xFF05 //Timer Counter
#define TMA 0xFF06 //Timer Modulo
#define TAC 0xFF07 //Timer Control

#define DMA 0xFF46 //OAM DMA transfer register



//Audio Registers
#define NR52 0xFF26 //Audio master control
#define NR51 0xFF25 //Sound panning
#define NR50 0xFF24 //Master volume and VIN panning

#define IF 0xFF0F //Interrupt Flag Register
#define IE 0xFFFF //Interrupt Enable Register



#define READ_ACCESS  0b00000010
#define WRITE_ACCESS 0b00000001
typedef enum {
    READ_WRITE = 0b00000011,
    READ_ONLY  = 0b00000010,
    WRITE_ONLY = 0b00000001,
    NO_ACCESS  = 0b00000000
} MemAccess;


void memory_init();
void memory_update();
uint8_t memory_get(uint16_t address);
void memory_set(uint16_t address, uint8_t value);
void memory_inc(uint16_t address);
void memory_set_access(MemAccess access, uint16_t address);
void memory_set_access_block(MemAccess access, uint16_t start, uint16_t end);
uint8_t memory_get_admin(uint16_t address);

#endif // !MEMORY_H
