#include "ppu.h"

#define TRANSPARENT 0
#define WHITE 0
#define LIGHT_GRAY 1
#define DARK_GRAY 2
#define BLACK 3

uint32_t color_map[4] = {
    0x9BBC0FFF, //White
    0x8BAC0FFF, //Light Gray
    0x306230FF, //Dark Gray
    0x0F380FFF  //Black
};




uint32_t framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH]; //Height x Width
uint8_t color_indices[SCREEN_HEIGHT][SCREEN_WIDTH]; //used to see if BG or Window have priority over OBJ

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

typedef struct {
    uint8_t y, x, index;
    union {
        uint8_t flags;
        struct{
            uint8_t cbg_palette : 3;
            uint8_t bank : 1;
            uint8_t dmg_palette : 1;
            uint8_t x_flip : 1;
            uint8_t y_flip : 1;
            uint8_t priority : 1;
        };
    };
} ObjAttribute;

ObjAttribute priority_objects[10];
uint8_t obj_num = 0;
uint16_t obj_height = 0;

void oamSearch() {
    uint8_t lcdc = memory_get(LCDC);
    uint8_t ly = memory_get(LY);
    obj_num = 0;
    obj_height = (lcdc & 0b00000100) ? 16 : 8;
    //OAM Search

    //Loop through all objects in OAM, which contains 40 entries
    //Identify the 10 objects allowed to render in this line, if they exist
    for(uint8_t i=0; i<40; i++) {
        uint16_t addr = OAM + i * 4;
        ObjAttribute obj = {memory_get_admin(addr), memory_get_admin(addr+1), memory_get_admin(addr+2), memory_get_admin(addr+3)}; 
        uint8_t screen_y = obj.y-16; //this will wrap around, but that should be fine in this case since the screen isn't that tall
        if(ly >= screen_y && ly <= screen_y + obj_height - 1) {
            priority_objects[obj_num] = obj;
            obj_num++;
            if(obj_num >= 10) {
                break;
            }
        }
    }

    //priority is decided by smallest x position.  if there is a tie, earliest OAM position wins
    //This means a stable sorting algorithm can be used since the above array is already sorted by OAM position
    //A stable algorithm will maintain the OAM position between objects of the same x value
    //Below is an implementation of Insertion Sort, which is simple and ideal for this application
    
    for(uint8_t i=1; i<obj_num; i++) {
        ObjAttribute temp = priority_objects[i];
        uint8_t j = i;
        while(j > 0 && priority_objects[j-1].x > temp.x) {
            priority_objects[j] = priority_objects[j-1];
            j--;
        }
        priority_objects[j] = temp;
    }
}


// draw current line to framebuffer
void drawLine() {
    uint8_t lcdc = memory_get(LCDC); //LCD Control

    uint8_t ly = memory_get(LY);//LCD Y Position



    //Skip BG and Window layers and set to white if flag is set
    if(!(lcdc & 0b01)) { //BG and Window enable/priority
        for(uint8_t x=0; x<SCREEN_WIDTH; x++) {
            for(uint8_t y=0; y<SCREEN_HEIGHT; y++) {
                framebuffer[y][x] = color_map[WHITE];
            }
        }
        goto Object;
    }

    TileMode tile_data_mode = (lcdc & 0b00010000) ? MODE_8000 : MODE_9000; //used for both window and bg


    ////////////////////////////////////////
    ///////////// BACKGROUND ///////////////
    ////////////////////////////////////////
//Background:

    uint8_t scy = memory_get(SCY);//Scroll Y
    uint8_t scx = memory_get(SCX);//Scroll X


    uint16_t tile_map_base_address =  (lcdc & 0b00001000) ? 0x9C00 : 0x9800;

    uint16_t win_tile_map_base_address =  (lcdc & 0b01000000) ? 0x9C00 : 0x9800;
    

    //get background palette
    uint8_t palette_reg = memory_get(BGP);
    uint8_t palette[4] = {palette_reg & 0b11, (palette_reg >> 2) & 0b11, (palette_reg >> 4) & 0b11, (palette_reg >> 6) & 0b11};


    uint8_t scroll_y = ly + scy;  //pixels scrolled on the y-axis. Same as: (ly + (uint16_t)scy) % 256
    uint8_t tile_y = scroll_y / 8; //tile coordinates
    uint8_t pix_row = scroll_y % 8; //pixel coordinates within the tile
    
    // the current row in the tile used for addressing
    uint16_t tile_row_offset = pix_row << 1; //pix_row * 2

    // Loop through each tile in the row plus one since scrolling means two partial tiles
    for (uint8_t i=0; i<SCREEN_WIDTH/8 + 1; i++) {
        uint8_t scroll_x = i*8 + scx; // pixels scrolled on the x-axis. Same as: (i*8 + (uint16_t)scx) % 256

        //tile coordinates
        uint8_t tile_x = scroll_x / 8;
        

        //pixel coordinates within the tile
        uint8_t pix_col = scroll_x % 8;

        uint8_t address = memory_get_admin(tile_map_base_address + tile_y * 32 + tile_x);

        uint16_t final_address = 0; 
        if(tile_data_mode == MODE_8000) {
            // Use Block 0 and 1, unsigned addressing, $8000 as base pointer
            final_address = 0x8000 + ((uint16_t)address) * 16;
        } else {
            // Use Block 1 and 2, signed addressing, $9000 as base pointer
            final_address = 0x9000 + ((int8_t)address) * 16;
        }
        
        //get bytes for the specific row for this tile
        register uint8_t low_byte = memory_get_admin(final_address + tile_row_offset);
        register uint16_t high_byte = ((uint16_t)memory_get_admin(final_address + tile_row_offset + 1)) << 1;
  

        //loop through each pixel in this row of the tile and draw to framebuffer
        //optimized for performance
        for(uint8_t p=0; p<=7; p++) {
            int screen_x = i*8 + (7-p) - pix_col;
            
            if(screen_x >= 0 && screen_x < SCREEN_WIDTH) {
                uint8_t color = ((low_byte & 0b01) | (high_byte & 0b10));
                framebuffer[ly][screen_x] = color_map[palette[color]];
                color_indices[ly][screen_x] = color;
            }
            low_byte = low_byte >> 1;
            high_byte = high_byte >> 1;
        }

    }


    ////////////////////////////////////////
    /////////////// WINDOW /////////////////
    ////////////////////////////////////////
    //TODO
    //code is mostly copied from background, reasonable optimization would be to combine with the background code
//Window:

    //skip if window not enabled
    if(!(lcdc & 0b00100000)) { //Window enable
        goto Object;
    }

    uint8_t wy = memory_get(WY);//Scroll Y
    uint8_t wx = memory_get(WX);//Scroll X

    tile_map_base_address =  (lcdc & 0b01000000) ? 0x9C00 : 0x9800;

    scroll_y = ly + wy;  //pixels scrolled on the y-axis. Same as: (ly + (uint16_t)scy) % 256
    tile_y = scroll_y / 8; //tile coordinates
    pix_row = scroll_y % 8; //pixel coordinates within the tile
    
    // the current row in the tile used for addressing
    tile_row_offset = pix_row << 1; //pix_row * 2

    // Loop through each tile in the row plus one since scrolling means two partial tiles
    for (uint8_t i=0; i<SCREEN_WIDTH/8 + 1; i++) {
        uint8_t scroll_x = i*8 + wx - 7; // pixels scrolled on the x-axis. Same as: (i*8 + (uint16_t)scx) % 256

        //tile coordinates
        uint8_t tile_x = scroll_x / 8;
        

        //pixel coordinates within the tile
        uint8_t pix_col = scroll_x % 8;

        uint8_t address = memory_get_admin(tile_map_base_address + tile_y * 32 + tile_x);

        uint16_t final_address = 0; 
        if(tile_data_mode == MODE_8000) {
            // Use Block 0 and 1, unsigned addressing, $8000 as base pointer
            final_address = 0x8000 + ((uint16_t)address) * 16;
        } else {
            // Use Block 1 and 2, signed addressing, $9000 as base pointer
            final_address = 0x9000 + ((int8_t)address) * 16;
        }
        
        //get bytes for the specific row for this tile
        register uint8_t low_byte = memory_get_admin(final_address + tile_row_offset);
        register uint16_t high_byte = ((uint16_t)memory_get_admin(final_address + tile_row_offset + 1)) << 1;
  

        //loop through each pixel in this row of the tile and draw to framebuffer
        //optimized for performance
        for(uint8_t p=0; p<=7; p++) {
            int screen_x = i*8 + (7-p) - pix_col;
            
            if(screen_x >= 0 && screen_x < SCREEN_WIDTH) {
                uint8_t color = ((low_byte & 0b01) | (high_byte & 0b10));
                framebuffer[ly][screen_x] = color_map[palette[color]];
                color_indices[ly][screen_x] = color;
            }
            low_byte = low_byte >> 1;
            high_byte = high_byte >> 1;
        }

    }


    ////////////////////////////////////////
    /////////////// OBJECT /////////////////
    ////////////////////////////////////////
    //Really the sprite layer, but nintendo is special so they're "objects"

Object:
    //skip if objects not enabled
    if(!(lcdc & 0b00000010)) { //Obj enable
        return;
    }


    if(obj_num == 0) { //skip if no objects
        return;
    }



    for(int8_t i=obj_num-1; i>=0; i--) {
        if(priority_objects[i].x == 0 || priority_objects[i].x >= 168) {continue;} //object is offscreen

        //screen coordinates
        uint8_t obj_y = priority_objects[i].y - 16;
        uint8_t obj_x = priority_objects[i].x - 8;


        //pixel coordinates within the tile
        uint8_t pix_row;
        if(priority_objects[i].y_flip) {
            pix_row = obj_height - 1 - (ly - obj_y);
        } else {
            pix_row = ly - obj_y;
        }
        

        uint16_t tile_row_offset = pix_row * 2;

        uint16_t final_address = 0x8000 + ((uint16_t)priority_objects[i].index) * 16;
        

        //get bytes for the specific row for this tile
        register uint8_t low_byte = memory_get_admin(final_address + tile_row_offset);
        register uint16_t high_byte = ((uint16_t)memory_get_admin(final_address + tile_row_offset + 1)) << 1;


        uint8_t obj_palette_reg = memory_get_admin((priority_objects[i].dmg_palette) ? OBP1 : OBP0);
        uint8_t obj_palette[4] = {obj_palette_reg & 0b11, (obj_palette_reg >> 2) & 0b11, (obj_palette_reg >> 4) & 0b11, (obj_palette_reg >> 6) & 0b11};
  

        //loop through each pixel in this row of the tile and draw to framebuffer
        //optimized for performance
        for(uint8_t p=0; p<=7; p++) {
            uint8_t pix;
            if(priority_objects[i].x_flip) { //flip x
                pix = p;
            } else {
                pix = (7-p);
            }
            int screen_x = obj_x + pix;
            
            if(screen_x >= 0 && screen_x < SCREEN_WIDTH) {
                uint8_t color = ((low_byte & 0b01) | (high_byte & 0b10));
                
                if(color != TRANSPARENT) {
                    if(priority_objects[i].priority) {
                        if(color_indices[ly][screen_x] == 0) { //indicies of 1-3 have priority over obj
                            framebuffer[ly][screen_x] = color_map[obj_palette[color]];
                        }
                    } else {
                        framebuffer[ly][screen_x] = color_map[obj_palette[color]];
                    }
                    
                }
                
            }
            low_byte = low_byte >> 1;
            high_byte = high_byte >> 1;
        }

    }

    return;

}


typedef enum {
    M_OAM=2,
    M_VRAM=3,
    M_HBLANK=0,
    M_VBLANK=1
} PPU_MODE;
PPU_MODE ppu_mode = M_OAM;

uint16_t ppu_clock = 0;



void updateSTAT() {
    uint8_t stat = memory_get(STAT);
    stat = (stat & ~0x03) | ppu_mode;
    memory_set(STAT, stat);
}

void ppu_init() {
    ppu_clock = 0;
    ppu_mode = M_OAM;


    memory_set(LCDC, 0x91);
    memory_set(STAT, 0x85);
    memory_set(SCY, 0x00);
    memory_set(SCX, 0x00);
    memory_set(LY, 0x00);
    memory_set(BGP, 0xFC);



    //memory_set(LY, 0x90);

}

void ppu_update() {
    ppu_clock++;

    uint8_t ly = memory_get(LY);//LCD Y Position
    uint8_t lyc = memory_get(LYC);//LY Compare

    uint8_t stat = memory_get(STAT);
    if(ly == lyc) {  //trigger interrupt if LY=LYC
        stat |= 0b00000100; //Set the LY=LYC flag
        if(stat & 0b01000000) trigger_interrupt(INT_LCD_STAT);
    } else {
        stat &= 0b11111011; //Clear the LY=LYC flag
    }
    memory_set(STAT, stat);
    
    //printf("PPU Clock: %d Mode: %d LY: %d\n", ppu_clock, ppu_mode, memory_get(LY));
    //Backgound


    switch(ppu_mode) {
        case M_OAM:
            if(ppu_clock >= 80) {
                oamSearch();
                ppu_mode = M_VRAM;
                ppu_clock -= 80;
                updateSTAT();
                //block VRAM accesss:
                //memory_set_access_block(NO_ACCESS, V_RAM, 0x9FFF);
            }
            break;
        case M_VRAM:
            if(ppu_clock >= 172) {
                ppu_mode = M_HBLANK;
                ppu_clock -= 172;
                drawLine();
                updateSTAT();
                //make VRAM accessible:
                //memory_set_access_block(READ_WRITE, V_RAM, 0x9FFF);
                //make OAM accessible:
                memory_set_access_block(READ_WRITE, OAM, 0xFE9F);
            }
            break;
        case M_HBLANK:
            if(ppu_clock >= 204) {
                ppu_clock -= 204;
                memory_set(LY, memory_get(LY) + 1);
                if(memory_get(LY) == 143) {
                    ppu_mode = M_VBLANK;
                    trigger_interrupt(INT_VBLANK);
                    platform_video_draw((uint32_t*)framebuffer);
                } else {
                    ppu_mode = M_OAM;
                    //block OAM accesss:
                    memory_set_access_block(NO_ACCESS, OAM, 0xFE9F);
                }
                updateSTAT();
            }

            break;
        case M_VBLANK:
            if(ppu_clock >= 456) {
                ppu_clock -= 456;
                memory_set(LY, memory_get(LY) + 1);

                if(memory_get(LY) > 153) {
                    memory_set(LY, 0);
                    ppu_mode = M_OAM;
                    //block OAM accesss:
                    memory_set_access_block(NO_ACCESS, OAM, 0xFE9F);
                }
                updateSTAT();
            }
            break;
        default:
            break;
    }
    //memory_set(LY, 0x90);
}