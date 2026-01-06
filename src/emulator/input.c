#include "input.h"

void input_update() {
    static uint32_t in_cnt = 70224;
    in_cnt++;

    static uint8_t prev_input = 0xFF;
    static uint8_t input = 0xFF;
    if(in_cnt >= 70224) {
        prev_input = input;
        input = platform_get_input();
        in_cnt=0;
    }
    
    uint8_t joyp = memory_get(JOYPAD);
    if(!(joyp & 0b00100000)) { //Select buttons
        uint8_t buttons = input & 0x0F;
        memory_set(JOYPAD, (joyp & 0xF0) | buttons);
        if((prev_input & 0x0F) & ~buttons) { //if button pressed
            trigger_interrupt(INT_JOYPAD);
        }

    } else if (!(joyp & 0b00010000)) { //Select D-pad
        uint8_t dpad = (input >> 4) & 0x0F;
        memory_set(JOYPAD, (joyp & 0xF0) | dpad);
        if(((prev_input >> 4) & 0x0F) & ~dpad)  { // if button pressed
            trigger_interrupt(INT_JOYPAD);
        }

    } else {
        memory_set(JOYPAD, joyp | 0b00001111);
    }

}