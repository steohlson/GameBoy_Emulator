#include <stdio.h>

#include "platform.h"

#include "emulator/memory.h"
#include "emulator/cpu.h"
#include "emulator/ppu.h"
#include "emulator/apu.h"
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

        #ifdef DEBUG_MODE
        uint64_t start_time = platform_time_ns();
        #endif

        cpu_update();

        #ifdef DEBUG_MODE
        uint64_t end_time = platform_time_ns();
        uint64_t delta_time = end_time - start_time;
        
        debug_output->NS_CPU = (float)delta_time;
        #endif
        clk_div =0;

        
    }
    #ifdef DEBUG_MODE
    uint64_t start_time = platform_time_ns();
    #endif

    ppu_update();

    #ifdef DEBUG_MODE
        uint64_t end_time = platform_time_ns();
        uint64_t delta_time = end_time - start_time;
        
        debug_output->NS_PPU = (float)delta_time;
    #endif

    apu_update();


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
    apu_init();
    #ifdef DEBUG_MODE
    debugLogInit();
    #endif // DEBUG_MODE
    

    while(1) {
        uint64_t frame_start_time = platform_time_ns();
        for(uint32_t i=0; i < CYCLES_PER_FRAME; i++) {
            #ifdef DEBUG_MODE
            uint64_t start_time = platform_time_ns();
            #endif


            sys_clk(); // system clock
            

            #ifdef DEBUG_MODE
            uint64_t end_time = platform_time_ns();
            uint64_t delta_time = end_time - start_time;
            debug_output->NS_TAKEN = (float)delta_time;
            debug_output->NS_OVER = (float)delta_time - (float)P_CPU_NS;
            debugLog();
            #endif // DEBUG_MODE

        }
        platform_update();
        uint64_t frame_delta_time = platform_time_ns() - frame_start_time;
        if(frame_delta_time < FRAME_TIME_NS) {
            platform_sleep_ns(FRAME_TIME_NS - frame_delta_time);
        }
        
    }
    platform_cleanup();
    return 0;
}
