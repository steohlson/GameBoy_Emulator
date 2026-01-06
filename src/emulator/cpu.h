#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdint.h>
#include "memory.h"
#include "platform.h"
#include <stdbool.h>
#include "../platform/debug.h"

void cpu_init();

//Update the CPU state for one cycle
void cpu_update();




void (*instructions[256])();
void (*cb_instructions[256])();

#endif // !CPU_H