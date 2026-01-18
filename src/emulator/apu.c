#include "apu.h"

bool apu_on = true;

bool ch1_on, ch2_on, ch3_on, ch4_on;
uint8_t ch1_ln_cnt, ch2_ln_cnt, ch4_ln_cnt;
uint16_t ch3_ln_cnt;

float buf_left[2048];
float buf_right[2048];

void apu_init() {
    apu_on = true;

    ch1_on = false;
    ch1_ln_cnt = 0;
    ch2_on = false;
    ch2_ln_cnt = 0;
    ch3_on = false;
    ch3_ln_cnt = 0;
    ch4_on = false;
    ch4_ln_cnt = 0;

    //memory_set_access(WRITE_ONLY, NR13);
    //memory_set_access(WRITE_ONLY, NR23);
    memory_set_access(WRITE_ONLY, NR31);
    //memory_set_access(WRITE_ONLY, NR33);
}

void div_apu() {
    static uint8_t cnt = 0;
    cnt++;
    if(cnt % 2 == 0) { //256 hz
        //sound length
        if(ch1_ln_cnt >= 64) {
            ch1_on = false;
        } else {
            ch1_ln_cnt++;
        }

        if(ch2_ln_cnt >= 64) {
            ch2_on = false;
        } else {
            ch2_ln_cnt++;
        }

        if(ch3_ln_cnt >= 256) {
            ch3_on = false;
        } else {
            ch3_ln_cnt++;
        }

        if(ch4_ln_cnt >= 64) {
            ch4_on = false;
        } else {
            ch4_ln_cnt++;
        }

    }
    if(cnt % 4 == 0) {
        //ch1 freq sweep
    }
    if(cnt % 8 == 0) {
        //envelope sweep
    }
    return;
}

//get next sample from channel 1
float channel_1() {
    uint8_t nr10 = memory_get_admin(NR10);
    uint8_t nr11 = memory_get_admin(NR11);
    uint8_t nr12 = memory_get_admin(NR12);
    uint8_t nr13 = memory_get_admin(NR13);
    uint8_t nr14 = memory_get_admin(NR14);

    if(!ch1_on) {
        return 0;
    }
}

//get next sample from channel 2
float channel_2() {
    uint8_t nr21 = memory_get_admin(NR21);
    uint8_t nr22 = memory_get_admin(NR22);
    uint8_t nr23 = memory_get_admin(NR23);
    uint8_t nr24 = memory_get_admin(NR24);

    //trigger
    if(nr24 & 0b10000000) {//this possibly should happen in memory
        ch2_on = true;
        //reset length timer
        ch2_ln_cnt = 0;
        //period divider
        //envelope timer reset
        //volume is set to contents of NR12
        //sweep apparently does things
        

    }

    if(!ch2_on) {
        return 0;
    }
}





void apu_update() {
    static uint32_t div_apu_cnt = 0;
    if(div_apu_cnt >= F_CPU / 512) {
        div_apu();
    }

    uint8_t master_ctrl = memory_get_admin(NR52);

    if(!(master_ctrl & 0b10000000)) {
        if(apu_on) {
            apu_on = false;
            //turning apu off clears all apu registers
            memory_set(NR51, 0);
            memory_set(NR50, 0);

            memory_set(NR10, 0);
            memory_set(NR11, 0);
            memory_set(NR12, 0);
            memory_set(NR13, 0);
            memory_set(NR14, 0);

            memory_set(NR21, 0);
            memory_set(NR22, 0);
            memory_set(NR23, 0);
            memory_set(NR24, 0);

            memory_set(NR30, 0);
            memory_set(NR31, 0);
            memory_set(NR32, 0);
            memory_set(NR33, 0);
            memory_set(NR34, 0);

            memory_set(NR41, 0);
            memory_set(NR42, 0);
            memory_set(NR43, 0);
            memory_set(NR44, 0);

            //set to read only access
            memory_set_access_block(READ_ONLY, 0xFF10, 0xFF14);
            memory_set_access_block(READ_ONLY, 0xFF16, 0xFF1E);
            memory_set_access_block(READ_ONLY, 0xFF20, 0xFF26);
        }
        return;
    }

    if(!apu_on) {
        apu_on = true;
        //restore read write access
        memory_set_access_block(READ_WRITE, 0xFF10, 0xFF14);
        memory_set_access_block(READ_WRITE, 0xFF16, 0xFF1E);
        memory_set_access_block(READ_WRITE, 0xFF20, 0xFF26);
        //memory_set_access(WRITE_ONLY, NR13);
        //memory_set_access(WRITE_ONLY, NR23);
        memory_set_access(WRITE_ONLY, NR31);
        //memory_set_access(WRITE_ONLY, NR33);
    }

    //maintains timing, all proceeding code runs at 48000 fps
    static double accum_cycles = 0;
    accum_cycles += 1;
    if(!(accum_cycles >= CYCLES_PER_SAMPLE)) {
        return;
    }
    accum_cycles -= CYCLES_PER_SAMPLE;
    
    //set channel on registers
    uint8_t ch_status = ch1_on | (ch2_on << 1) | (ch3_on << 2) | (ch4_on << 3);
    memory_set(NR52, (master_ctrl & 0b10000000) | ch_status);

}