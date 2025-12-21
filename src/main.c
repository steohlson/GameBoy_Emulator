#include <stdio.h>

#include "platform.h"

#include "emulator/memory.h"
#include "emulator/cpu.h"
#include "emulator/ppu.h"
#include "emulator/gb.h"
#include "emulator/input.h"


/*
System clock function
*/
void sys_clk() {
    input_update();
    cpu_update();
    //ppu_update();
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
    cpu_init();
    //ppu_init();

    while(1) {
        uint64_t start_time = platform_time_ns();

        sys_clk(); // system clock

        uint64_t end_time = platform_time_ns();
        uint64_t delta_time = end_time - start_time;
        if(delta_time <= P_CPU_NS / 2) {
            platform_sleep_ns((uint64_t)(P_CPU_NS / 2) - (delta_time));
        }

        start_time = platform_time_ns();

        sys_clk_inv(); // inverse of system clock

        end_time = platform_time_ns();
        delta_time = end_time - start_time;
        if(delta_time <= P_CPU_NS / 2) {
            platform_sleep_ns((uint64_t)(P_CPU_NS / 2) - (delta_time));
        }
    }
    return 0;
}
