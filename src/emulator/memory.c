#include "memory.h"

#include "bootrom.h"


uint8_t memory[65536];

uint8_t memory_access[65536];


bool boot_rom_enabled;

//OAM DMA transfer variables
uint16_t dma_clock = 0;
bool dma_activated = false;

void memory_init() {
    memset(memory, 0, sizeof(memory));
    memset(memory_access, READ_WRITE, sizeof(memory_access));
    //for(int i=0; i < sizeof(memory) / sizeof(memory[0]); i++)     { memory[i] = 0; }
    boot_rom_enabled = true;
    dma_clock = 0;
    dma_activated = false;
}


//mainly for DMA transfer
void memory_update() {
    if(dma_activated) {
        dma_clock++;
        if(dma_clock >= 640) {
            //Move memory
            uint16_t src_start = memory_get(DMA) << 8;
            if(src_start > 0xDF00) {src_start = 0xDF00;} //clamp to valid range
            //uint16_t src_end = src_start | 0x009F;
            //uint16_t dest_start = 0xFE00;
            //uint16_t dest_end   = 0xFR9F;
            if(src_start < 0x8000) { // in cartridge memory
                uint16_t src_address = src_start + cartridge_offset;
                if(src_address + 40*4 < cartridge_rom.size) {
                    memcpy(&memory[0xFE00], &cartridge_rom.data[src_address], 40 * 4);
                } else {
                    printf("Error: DMA attempted to copy outside of cartridge rom size");
                }
                
            } else { // in gameboy memory
                memcpy(&memory[0xFE00], &memory[src_start], 40 * 4);
            }
            //deactivate dma transfer
            dma_activated = false;
            dma_clock = 0;
            //memory_set(DMA, 0);
        }
    }
}


uint8_t memory_get(uint16_t address) {
    if(!(memory_access[address] & READ_ACCESS)) {
        return 0xFF;
    }
    if(address < 0x8000) {
        if(address < 0x0100 && boot_rom_enabled) {
            return dmg_boot_bin[address];
        }
        return cartridge_get(address);
    }
    return memory[address];
}

void memory_set(uint16_t address, uint8_t value) {
    if(!(memory_access[address] & WRITE_ACCESS)) {
        return;
    }
    if(address == DMA) {
        dma_activated = true;
    }
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


void memory_set_access(MemAccess access, uint16_t address) {
    memory_access[address] = (uint8_t)access;
}

void memory_set_access_block(MemAccess access, uint16_t start, uint16_t end) {
    memset(&memory_access[start], (uint8_t)access, end - start + 1);
    //for(uint16_t i = start; i <= end; i++) {
    //    memory_access[i] = (uint8_t)access;
    //}
}

//memory_get without access restrictions
uint8_t memory_get_admin(uint16_t address) {
    if(address < 0x8000) {
        if(address < 0x0100 && boot_rom_enabled) {
            return dmg_boot_bin[address];
        }
        return cartridge_get(address);
    }
    return memory[address];
}