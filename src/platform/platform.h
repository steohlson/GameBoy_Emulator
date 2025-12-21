#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

void platform_init();
void platform_video_draw(const uint32_t *framebuffer);
void platform_audio_play(float* samples, size_t count);



#define IN_DOWN   (1<<7)
#define IN_UP     (1<<6)
#define IN_LEFT   (1<<5)
#define IN_RIGHT  (1<<4)
#define IN_START  (1<<3)
#define IN_SELECT (1<<2)
#define IN_B      (1<<1)
#define IN_A      (1<<0)
// returns 8-bit value: Down Up Left Right Start Select B A
// 0 means pressed
uint8_t platform_get_input();

uint64_t platform_time_ns();
void platform_sleep_ns(uint64_t ns);
void platform_log(const char* buf);



#endif // !PLATFORM_H