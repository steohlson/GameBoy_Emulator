#include "timer.h"
#include "platform.h"

/*
System clock function
*/
void sys_clk() {
    printf("1");
}

/*
Some peripherals use the inverted system clock
*/
void sys_clk_inv() {
    printf("0");
}

/*
Initialize timer
*/
void timer_init() {

}

/*
Update timer
*/
void timer_update() {
    uint64_t start_time = platform_time_ns();

    sys_clk(); // system clock

    uint64_t end_time = platform_time_ns();
    uint64_t delta_time = end_time - start_time;
    if(delta_time <= P_CPU_NS / 2) {
        platform_sleep_ns((P_CPU_NS / 2) - (delta_time));
    }

    start_time = platform_time_ns();

    sys_clk_inv(); // inverse of system clock

    end_time = platform_time_ns();
    delta_time = end_time - start_time;
    if(delta_time <= P_CPU_NS / 2) {
        platform_sleep_ns((P_CPU_NS / 2) - (delta_time));
    }

}