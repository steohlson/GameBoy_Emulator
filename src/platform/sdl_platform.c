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


    //Set window icon
    SDL_Surface *icon = SDL_LoadBMP("icon.bmp");
    if(icon) {
        SDL_SetWindowIcon(window, SDL_LoadBMP("icon.bmp"));
    }
    


    SDL_AudioSpec spec;
    spec.channels = 2;
    spec.format = SDL_AUDIO_F32;
    spec.freq = (int)F_AUDIO_SAMPLE;
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(stream);

}

void platform_update() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_EVENT_QUIT) {
            exit(0);
        }
    }
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

void platform_audio_play(float *left, float *right, size_t count) {
    const void* planes[2] = {left, right};
    SDL_PutAudioStreamPlanarData(stream, planes, 2, (int)(count * sizeof(float)));
}


bool failed = false;
bool done = false;
char path[256];

void fileOpenCallback(void *userdata, const char * const *filelist, int filter) {
    Rom *rom = userdata;
    if(!filelist || !filelist[0]) {
        SDL_Log("No file selected");
        done = true;
        failed = true;
        return;
    }
    
    strcpy(path, filelist[0]);
    //path = filelist[0];
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
    SDL_DialogFileFilter filters[1];
    filters[0].name = "GameBoy ROMs";
    filters[0].pattern = "gb;gbc";

    SDL_ShowOpenFileDialog(fileOpenCallback, rom, window, filters, 1, NULL, false);
    
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

void platform_write_save(Rom *ram) {
    char save_path[256];//strcat(strcat(path, "/"), file_name);
    strcpy(save_path, path);
    strcat(save_path, ".sav");
    FILE *file = fopen(save_path, "wb");

    //if(!file) {
    //    SDL_Log("Failed to open file for writing: %s", path);
    //    return;
    //
    //}
    
    fwrite(ram->data, 1, ram->size, file);
    fclose(file);
}

void platform_load_save(Rom *ram) {
    //char* save_path = strcat(strcat(path, "/"), file_name);
    char save_path[256];//strcat(strcat(path, "/"), file_name);
    strcpy(save_path, path);
    strcat(save_path, ".sav");
    FILE *file = fopen(save_path, "rb");
    if(!file) {
        SDL_Log("Failed to open save file: %s", save_path);
        return;
    }
    SDL_Log("Loading save file: %s", save_path);

   
    fread(ram->data, 1, ram->size, file);
    fclose(file);
}


uint8_t platform_get_input() {

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

    if(keys[SDL_SCANCODE_RIGHT]) input &= ~IN_RIGHT;
    if(keys[SDL_SCANCODE_LEFT]) input &= ~IN_LEFT;
    if(keys[SDL_SCANCODE_UP]) input &= ~IN_UP;
    if(keys[SDL_SCANCODE_DOWN]) input &= ~IN_DOWN;

    if(keys[SDL_SCANCODE_RETURN]) {
        input &= ~IN_A;
        input &= ~IN_B;
        input &= ~IN_START;
        input &= ~IN_SELECT;
    }
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