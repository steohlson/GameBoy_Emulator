#ifndef GB_H
#define GB_H

#define F_CPU 4194304 // 4.194304 MHz
#define P_CPU_S ( 1.0 / ((float) F_CPU ) )
#define P_CPU_NS (P_CPU_S * 1000000000)

#define WIDTH 160
#define HEIGHT 144

typedef struct{
    uint8_t* data;
    size_t size;
} Rom;

#endif // !GB_H