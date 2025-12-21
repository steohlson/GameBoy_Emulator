#include "interrupt.h"

void trigger_interrupt(InterruptType type) {
    memory_set(IF, memory_get(IF) | (1 << type));
}