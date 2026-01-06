#ifndef PPU_H
#define PPU_H

#include "memory.h"
#include "../platform/platform.h"
#include "gb.h"
#include "interrupt.h"
#include "../platform/debug.h"

//LCD Registers
#define LCDC 0xFF40 //LCD Control
#define STAT 0xFF41 //LCD Status
#define LY 0xFF44 //LCD Y Position
#define LYC 0xFF45 //LY Compare
#define SCY 0xFF42 //Scroll Y
#define SCX 0xFF43 //Scroll X
#define WY 0xFF4A //Window Y Position
#define WX 0xFF4B //Window X Position
#define BGP 0xFF47 //Background Palette
#define OBP0 0xFF48 //Object Palette 0
#define OBP1 0xFF49 //Object Palette 1

void ppu_init();
void ppu_update();

#endif // !PPU_H