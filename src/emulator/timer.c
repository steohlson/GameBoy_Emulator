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
    uint64_t start_time = platform_time_ms();

}