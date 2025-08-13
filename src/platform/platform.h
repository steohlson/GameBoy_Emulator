#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include "gb.h"


void platform_init();
void platform_video_draw(const uint32_t *framebuffer);
void platform_audio_play(float* samples, size_t count);
uint8_t platform_get_input();
uint64_t platform_time_ms();
void platform_sleep_ms(uint32_t ms);
void platform_log(const char* buf);



#endif // !PLATFORM_H