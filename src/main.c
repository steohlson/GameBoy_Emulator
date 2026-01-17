#include <stdio.h>

#include "platform.h"

#include "emulator/memory.h"
#include "emulator/cpu.h"
#include "emulator/ppu.h"
#include "emulator/gb.h"
#include "emulator/timer.h"
#include "emulator/input.h"
#include "emulator/cartridge.h"
#include "platform/debug.h"


/*
System clock function
*/
void sys_clk() {
    static uint8_t clk_div = 0;
    clk_div++;
    timer_update();
    
    memory_update();
    
    if(clk_div >= 3) {
        input_update();
        uint64_t start_time = platform_time_ns();
        cpu_update();
        #ifdef DEBUG_MODE
        uint64_t end_time = platform_time_ns();
        uint64_t delta_time = end_time - start_time;
        
        debug_output->NS_CPU = (float)delta_time;
        #endif
        clk_div =0;

        
    }
    uint64_t start_time = platform_time_ns();
    ppu_update();
    #ifdef DEBUG_MODE
        uint64_t end_time = platform_time_ns();
        uint64_t delta_time = end_time - start_time;
        
        debug_output->NS_PPU = (float)delta_time;
    #endif
}

/*
Some peripherals use the inverted system clock
*/
void sys_clk_inv() {
    return;
}

int main(){
    platform_init();
    memory_init();
    cartridge_load();
    cpu_init();
    ppu_init();
    #ifdef DEBUG_MODE
    debugLogInit();
    #endif // DEBUG_MODE
    

    while(1) {
        uint64_t start_time = platform_time_ns();

        sys_clk(); // system clock

        uint64_t end_time = platform_time_ns();
        uint64_t delta_time = end_time - start_time;
        #ifdef DEBUG_MODE
        debug_output->NS_TAKEN = (float)delta_time;
        debug_output->NS_OVER = (float)delta_time - (float)P_CPU_NS;
        debugLog();
        #endif // DEBUG_MODE
        //if(delta_time < (uint64_t)P_CPU_NS) {
            //printf("Line took too long%f\n", (float)delta_time);// - P_CPU_NS);
            //platform_sleep_ns((uint64_t)(P_CPU_NS) - (delta_time));
        //}

        static uint16_t platform_cnt = 0;
        platform_cnt++;
        if(platform_cnt == 255) {
            platform_update();
        }
        
        //start_time = platform_time_ns();

        //sys_clk_inv(); // inverse of system clock

        //end_time = platform_time_ns();
        //delta_time = end_time - start_time;
        //if(delta_time <= P_CPU_NS / 2) {
            //platform_sleep_ns((uint64_t)(P_CPU_NS / 2) - (delta_time));
        //}
    }
    platform_cleanup();
    return 0;
}
