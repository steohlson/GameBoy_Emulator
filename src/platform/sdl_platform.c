#include "./platform.h"
#include "SDL3/SDL.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

static SDL_AudioStream *stream = NULL;



#define SCALE 6

void platform_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow("GameBoy Emulator", SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, 0);
    renderer = SDL_CreateRenderer(window, NULL);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

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
    uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    for(int i=0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        pixels[i] = framebuffer[i];
    } 
    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void platform_audio_play(float* samples, size_t count) {
    SDL_PutAudioStreamData(stream, samples, (int)(count * sizeof(float)));
}


bool failed = false;
bool done = false;

void fileOpenCallback(void *userdata, const char * const *filelist, int filter) {
    Rom *rom = userdata;
    if(!filelist || !filelist[0]) {
        SDL_Log("No file selected");
        done = true;
        failed = true;
        return;
    }
    const char *path = filelist[0];
    SDL_Log("Loading file: %s", path);

    FILE *file = fopen(path, "rb");
    
    if(!file) {
        SDL_Log("Failed to open file: %s", path);
        done = true;
        failed = true;
        return;
    }
    
    fseek(file, 0 , SEEK_END);
    rom->size = ftell(file);
    rewind(file);
    rom->data = (uint8_t*)malloc(rom->size);
    fread(rom->data, 1, rom->size, file);
    fclose(file);
    done = true;
}

void platform_file_load(Rom *rom) {
    restart:
    failed = false;
    done = false;

    SDL_ShowOpenFileDialog(fileOpenCallback, rom, window, NULL, 0, NULL, false);
    
    while(!done) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) {
                exit(0);
            }
        }
        SDL_Delay(100);
    }

    if(failed) {
        goto restart;
    }

}



uint8_t platform_get_input() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_EVENT_QUIT) {
            exit(0);
        }
    }

    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    uint8_t input = 0xFF;

    if(keys[SDL_SCANCODE_Q]) input &= ~IN_A;
    if(keys[SDL_SCANCODE_E]) input &= ~IN_B;
    if(keys[SDL_SCANCODE_LSHIFT]) input &= ~IN_SELECT;
    if(keys[SDL_SCANCODE_TAB]) input &= ~IN_START;
    if(keys[SDL_SCANCODE_D]) input &= ~IN_RIGHT;
    if(keys[SDL_SCANCODE_A]) input &= ~IN_LEFT;
    if(keys[SDL_SCANCODE_W]) input &= ~IN_UP;
    if(keys[SDL_SCANCODE_S]) input &= ~IN_DOWN;

    return input;
}

uint64_t platform_time_ns() {
    return SDL_GetTicksNS();
}

void platform_sleep_ns(uint64_t ns) {
    SDL_DelayNS(ns);
}

void platform_log(const char* buf) {
    SDL_Log(buf);
}

void platform_cleanup() {

    if(texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    if(renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if(window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}