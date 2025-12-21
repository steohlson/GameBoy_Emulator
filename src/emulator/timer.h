#ifndef TIMER_H
#define TIMER_H

#include "platform.h"
#include "gb.h"
#include "memory.h"
#include "interrupt.h"
#include <math.h>



#define F_DIV 16384 //Hz
#define CYCLES_PER_DIV (F_CPU / F_DIV) //256



void timer_update();


#endif // !TIMER_H

