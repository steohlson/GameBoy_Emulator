#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "memory.h"

typedef enum {
    INT_VBLANK = 0,
    INT_LCD_STAT = 1,
    INT_TIMER = 2,
    INT_SERIAL = 3,
    INT_JOYPAD = 4
} InterruptType;

void trigger_interrupt(InterruptType type);

#endif // !INTERRUPT_H
