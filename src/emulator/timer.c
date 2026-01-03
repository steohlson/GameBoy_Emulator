#include "timer.h"

static const uint16_t TAC_m_cycles[4] = { 256, 4, 16, 64 }; //Timer Control Register Options

void timer_update() {
    static uint16_t div_counter = 0;
    div_counter++;
    if (div_counter >= CYCLES_PER_DIV) {
        div_counter = 0;
        memory_inc(DIV);
    }

    static uint16_t tima_counter = 0;
    uint8_t tac = memory_get(TAC);

    if(tac & 0b00000100) {
        tima_counter++;


        if (tima_counter >= TAC_m_cycles[tac & 0b00000011]) {
            tima_counter -= TAC_m_cycles[tac & 0b00000011];
            uint8_t tima = memory_get(TIMA);
            if(tima == 0xFF) {
                memory_set(TIMA, memory_get(TMA));
                trigger_interrupt(INT_TIMER);
            } else {
                memory_set(TIMA, tima + 1);
            }
        }
    } else {
        tima_counter = 0;
    }
    
}