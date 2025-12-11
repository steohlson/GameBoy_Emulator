#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdint.h>
#include "memory.h"
#include "platform.h"
#include <stdbool.h>

void cpu_init();
uint8_t cpu_update();



void *instructions[];

#endif // !CPU_H