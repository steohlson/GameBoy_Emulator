#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "memory.h"
#include "platform.h"

void cpu_init();
uint8_t cpu_update();

#endif // !CPU_H