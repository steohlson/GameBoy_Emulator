#include <stdio.h>
#include "platform.h"
#include "emulator/timer.h"

int main(){
    platform_init();
    timer_init();

    while(1) {
        timer_update();
    }
    return 0;
}
