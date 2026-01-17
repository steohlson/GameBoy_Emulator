#include "memory.h"
#include "bootrom.h"
#include "cartridge.h"


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
    } else if(address >= 0xA000 && address < 0xC000) { //External RAM
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
    if(address <= 0x8000 || (address >= 0xA000 && address < 0xC000)) {
        cartridge_set(address, value);
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
    } else if(address >= 0xA000 && address < 0xC000) { //External RAM
        return cartridge_get(address);
    }
    return memory[address];
}



/////////////////////
//////Cartridge//////
/////////////////////


Rom cartridge_rom = {0};
uint16_t cartridge_offset = 0;

uint8_t mbc_type;

uint16_t rom_size = 0; // number of rom banks
uint16_t ram_size = 0; // number or ram banks

bool ram_enable = false;
uint8_t rom_bank_num = 0;
uint8_t ram_bank_num = 0;
bool bank_mode_select;

uint16_t rom_mask = 0xFFFF; // a little complicated to explain, basically just a mask for all bits required to represent all banks

void cartridge_load() {
    ram_enable = false;
    rom_bank_num = 0;
    cartridge_offset = 0;
    platform_file_load(&cartridge_rom);
    mbc_type = cartridge_rom.data[0x0147];

    switch(mbc_type) {
        case ROM_ONLY:
            break;
        case MBC1:
            rom_size = ((uint16_t)2 << cartridge_rom.data[0x148]);
            ram_size = 0;
            break;

        case MBC1_RAM://either with or without battery, both do the same thing
        case MBC1_RAM_BATT:
            rom_size = ((uint16_t)2 << cartridge_rom.data[0x148]);
            //Set RAM size according to value in cartridge header
            //I'm gonna be honest, I'm not sure what nintendo was doin when they decided to define RAM size this way
            switch(cartridge_rom.data[0x149]) {
                case 0:
                case 1:
                    ram_size = 0;
                    break;
                case 2:
                    ram_size = 1;
                    break;
                case 3:
                    ram_size = 4;
                    break;
                case 4:
                    ram_size = 16;
                    break;
                case 5:
                    ram_size = 8;
            }

            //perform bit smearing to get only bits necessary for the rom size and ignore higher bits
            rom_mask = rom_size;
            rom_mask |= rom_mask >> 1;
            rom_mask |= rom_mask >> 2;
            rom_mask |= rom_mask >> 4;

            break;


        default:
            break;
    }
}

uint8_t cartridge_get(uint16_t address) {
    uint32_t final_address = address;
    uint32_t addr_mask = (rom_mask << 14) | 0b11111111111111;
    switch(mbc_type) {
        case ROM_ONLY:
            break;
        case MBC1:
        case MBC1_RAM:
        case MBC1_RAM_BATT:
            
            if(address <= 0x3fff) {
                //Mode 0 just sets to the address with 0s for all other bits
                if(bank_mode_select){ //Mode 1
                    //TODO check
                    final_address = (final_address & 0b11111111111111) | ((0b11 & (rom_bank_num >> 5)) << 19);

                }
            } else if(address <= 0x7fff) {
                final_address = (final_address & 0b11111111111111) | ((0b1100000 & (rom_bank_num)) << 14) & addr_mask;
            } else if(address <= 0xbfff) {
                //no change of final address in mode 0
                if(bank_mode_select){ //Mode 1
                    final_address = (final_address & 0b11111111111111) | ((0b11 & (rom_bank_num >> 5)) << 13);
                }
            }
            
            break;
        default:
            break;
    }

    if(final_address >= cartridge_rom.size) {
        return 0xFF;
    }
    return cartridge_rom.data[final_address];
}

void cartridge_set(uint16_t address, uint8_t value) {
    switch(mbc_type) {
        case ROM_ONLY:
            return;
        case MBC1:
        case MBC1_RAM:
        case MBC1_RAM_BATT:
            if(address <= 0x1FFF) { //RAM Enable
                if((value & 0x0F) == 0xA) { //If a value of A is written here it enables ram
                    ram_enable = true;
                } else {
                    ram_enable = false;
                }
                return;
            } else if(address <= 0x3FFF) { // ROM Bank Number
                //Sets the first 5 bits of the ROM bank register
                rom_bank_num = (0b11100000 & rom_bank_num) | (0b00011111 & value);
                if((0b00011111 & rom_bank_num) == 0) {
                    rom_bank_num = 1; //you cannot set bank 0 since its always accessible in 0000-3fff
                    //interestingly, on the gameboy you can get around this by messing with the following masking behavior
                    //assuming the rom is small enough
                    //that behavior is preserved in this implementation
                } else if ((0b00011111 & rom_bank_num) > rom_size) {

                    rom_bank_num &= rom_mask;

                }
                return;

            } else if ((0b00011111 & rom_bank_num) <= 0x5FFF) { // RAM Bank Number or upper bits of Rom bank num
                //TODO add support for MBC1M carts
                if(ram_size > 0) {
                    ram_bank_num = 0b11 & value;
                } else if(rom_size >= 64) {//1 MiB ROM or larger
                    rom_bank_num = (rom_bank_num & 0b00011111) | ((0b11 & value) << 5);
                }
                return;
            } else if ((0b00011111 & rom_bank_num) <= 0x7FFF) { // Banking Mode Select
                bank_mode_select = 0b1 & value;
            }
            break;


        default:
            break;
    }
    
    address += cartridge_offset;
    if(address >= cartridge_rom.size) {
        return;
    }
    cartridge_rom.data[address] = value;
}