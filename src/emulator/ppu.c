#include "ppu.h"


#define WHITE 0
#define LIGHT_GRAY 1
#define DARK_GRAY 2
#define BLACK 3

uint32_t color_map[4] = {
    0xFF9BBC0F, //White
    0xFF8BAC0F, //Light Gray
    0xFF306230, //Dark Gray
    0xFF0F380F  //Black
};


uint32_t framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH]; //Height x Width

typedef enum {
    BACKGROUND,
    WINDOW,
    OBJECT
} TileType;

typedef enum {
    MODE_8000 = 0, // LCDC.4 = 1, or Objects
    MODE_9000 = 1 // LCDC.4 = 0
} TileMode;

typedef struct {
    uint8_t data[16]; // Each tile is 16 bytes (8x8 pixels)
} Tile;


Tile getTile(uint8_t address, TileMode mode) {
    uint16_t final_address;
    if(mode == MODE_8000) {
        // Use Block 0 and 1, unsigned addressing, $8000 as base pointer
        final_address = 0x8000 + (uint16_t)address * 16;
    } else if(mode == MODE_9000) {
        // Use Block 1 and 2, signed addressing, $9000 as base pointer
        final_address = 0x9000 + (int8_t)address * (int)16;
    }
    Tile tile;
    for(int i = 0; i < 16; i++) {
        tile.data[i] = memory_get(final_address + i);
    }
    return tile;
}


void drawLine() {
    uint8_t lcdc = memory_get(LCDC); //LCD Control

    uint8_t ly = memory_get(LY);//LCD Y Position
    uint8_t lyc = memory_get(LYC);//LY Compare

    uint8_t scy = memory_get(SCY);//Scroll Y
    uint8_t scx = memory_get(SCX);//Scroll X

    uint8_t stat = memory_get(STAT);
    if(ly == lyc) {  //trigger interrupt if LY=LYC
        stat |= 0b00000100; //Set the LY=LYC flag
        
        if(stat & 0b01000000) trigger_interrupt(INT_LCD_STAT);
    } else {
        stat &= 0b11111011; //Clear the LY=LYC flag
    }
    memory_set(STAT, stat);


    uint8_t tile_map_base_address =  (lcdc & 0b00001000) ? 0x9C00 : 0x9800;
    TileMode tile_data_mode = (lcdc & 0b00010000) ? MODE_8000 : MODE_9000;


    uint8_t scroll_y = (ly + (uint16_t)scy) % 256;
    for (int i=0; i<SCREEN_WIDTH/8 + 1; i++) {
        uint8_t scroll_x = (i*8 + (uint16_t)scx) % 256;
        //tile coordinates
        uint8_t tile_x = scroll_x / 8;
        uint8_t tile_y = scroll_y / 8;

        //pixel coordinates within the tile
        uint8_t row = scroll_y % 8;
        uint8_t col = scroll_x % 8;

        uint8_t address = memory_get(tile_map_base_address + tile_y * 32 + tile_x);

        uint16_t final_address = 0; 
        if(tile_data_mode == MODE_8000) {
            // Use Block 0 and 1, unsigned addressing, $8000 as base pointer
            final_address = 0x8000 + (uint16_t)address * 16;
        } else if(tile_data_mode == MODE_9000) {
            // Use Block 1 and 2, signed addressing, $9000 as base pointer
            final_address = 0x9000 + (int8_t)address * (int)16;
        }
        
        uint8_t low_byte = memory_get(final_address + row * 2);
        uint8_t high_byte = memory_get(final_address + row * 2 + 1);


        uint8_t palette_reg = memory_get(BGP);
        uint8_t palette[4] = {palette_reg & 0b11, (palette_reg >> 2) & 0b11, (palette_reg >> 4) & 0b11, (palette_reg >> 6) & 0b11};


        for(int p=7; p>=0; p--) {
            int screen_x = i*8 + (7 - p) - col;
            if(screen_x >= 0 && screen_x < SCREEN_WIDTH) {
                uint8_t color = ((low_byte >> p) & 0b01) | (((high_byte >> p) & 0b01) << 1);
                framebuffer[ly][i*8 + (7 - p) - col] = color_map[palette[color]];
            }
        }

    }



}


typedef enum {
    OAM=2,
    VRAM=3,
    HBLANK=0,
    VBLANK=1
} PPU_MODE;
PPU_MODE ppu_mode = OAM;

uint16_t ppu_clock = 0;


void updateSTAT() {
    uint8_t stat = memory_get(STAT);
    stat = (stat & ~0x03) | ppu_mode;
    memory_set(STAT, stat);
}

void ppu_init() {
    ppu_clock = 0;
    ppu_mode = OAM;
    memory_set(LCDC, 0x91);
    memory_set(STAT, 0x85);
    memory_set(SCY, 0x00);
    memory_set(SCX, 0x00);
    memory_set(LY, 0x00);
    memory_set(BGP, 0xFC);


    
    memory_set(LY, 0x90);

}

void ppu_update() {
    ppu_clock++;
    //printf("PPU Clock: %d Mode: %d LY: %d\n", ppu_clock, ppu_mode, memory_get(LY));
    //Backgound


    switch(ppu_mode) {
        case OAM:
            if(ppu_clock >= 80) {
                ppu_mode = VRAM;
                ppu_clock -= 80;
                updateSTAT();
            }
            break;
        case VRAM:
            if(ppu_clock >= 172) {
                ppu_mode = HBLANK;
                ppu_clock -= 172;
                drawLine();
                updateSTAT();
            }
            break;
        case HBLANK:
            if(ppu_clock >= 204) {
                ppu_clock -= 204;
                memory_set(LY, memory_get(LY) + 1);
                if(memory_get(LY) == 143) {
                    ppu_mode = VBLANK;
                    trigger_interrupt(INT_VBLANK);
                    platform_video_draw((uint32_t*)framebuffer);
                } else {
                    ppu_mode = OAM;
                }
                updateSTAT();
            }

            break;
        case VBLANK:
            if(ppu_clock >= 456) {
                ppu_clock -= 456;
                memory_set(LY, memory_get(LY) + 1);

                if(memory_get(LY) > 153) {
                    memory_set(LY, 0);
                    ppu_mode = OAM;
                }
                updateSTAT();
            }
            break;
        default:
            break;
    }
    //memory_set(LY, 0x90);
}