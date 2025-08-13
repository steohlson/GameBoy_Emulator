#include "platform.h"
#include "SDL3/SDL.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

static SDL_AudioStream *stream = NULL;



#define SCALE 10

void platform_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow("GameBoy Emulator", WIDTH * SCALE, HEIGHT * SCALE, 0);
    renderer = SDL_CreateRenderer(&window, NULL);
    texture = SDL_CreateTexture(&renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    //Set scalemode to nearest, otherwise pixels will be blurred
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);


    SDL_AudioSpec spec;
    spec.channels = 1;
    spec.format = SDL_AUDIO_F32;
    spec.freq = 8192;
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(stream);

}


void platform_video_draw(const uint32_t *framebuffer) {
    uint32_t pixels[WIDTH * HEIGHT];
    for(int i=0; i < WIDTH * HEIGHT; i++) {
        pixels[i] = &framebuffer[i];
    } 
    SDL_UpdateTexture(texture, NULL, pixels, WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void platform_audio_play(float* samples, size_t count) {
    SDL_PutAudioStreamData(stream, samples, count);
}


uint8_t platform_get_input();

uint64_t platform_time_ms() {
    return SDL_GetTicks();
}

void platform_sleep_ms(uint32_t ms) {
    SDL_Delay(ms);
}

void platform_log(const char* buf) {
    SDL_Log(buf);
}