#ifndef GB_H
#define GB_H

#include <stdint.h>

#define F_CPU 4194304 // 4.194304 MHz
#define P_CPU_S ( 1.0 / ((float) F_CPU ) )
#define P_CPU_NS (P_CPU_S * 1000000000)

#define FPS 59.7275
#define CYCLES_PER_FRAME ((float)F_CPU / FPS)
#define FRAME_TIME_NS ((uint64_t)(1e9 / FPS))

#define WIDTH 160
#define HEIGHT 144

typedef struct{
    uint8_t* data;
    size_t size;
} Rom;

#endif // !GB_H