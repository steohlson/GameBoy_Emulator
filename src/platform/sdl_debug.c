#include "debug.h"
#include "SDL3/SDL.h"

FILE *cpu_log = NULL;

#define LOG_CAP 2048

DebugOutput *debug_output;

DebugOutput log_buf[LOG_CAP];

static SDL_AtomicInt write_idx;
static SDL_AtomicInt read_idx;

static SDL_Mutex *log_mutex;
static SDL_Condition *log_cond;
static SDL_Thread *log_thread;
static bool log_running = true;



void logToFile(DebugOutput *d) {
    fprintf(
        cpu_log,
        "%04X, "//PC
        "%04X, "//SP
        "%08X, "//MEM_PC
        "%04X, "//AF
        "%04X, "//BC
        "%04X, "//DE
        "%04X, "//HL
        "%u, "  //PPU_MODE
        "%f, "  //NS_TAKEN
        "%f, "  //NS_OVER
        "%f, "  //NS_CPU
        "%f\n", //NS_PPU
        d->PC,
        d->SP,
        d->MEM_PC,
        d->AF,
        d->BC,
        d->DE,
        d->HL,
        d->PPU_MODE,
        d->NS_TAKEN,
        d->NS_OVER,
        d->NS_CPU,
        d->NS_PPU
    );
}

int logger_thread(void *unused) {
    while(log_running) {
        SDL_LockMutex(log_mutex);
        while (SDL_GetAtomicInt(&read_idx) == SDL_GetAtomicInt(&write_idx) && log_running) {
            SDL_WaitCondition(log_cond, log_mutex);
        }

        while (SDL_GetAtomicInt(&read_idx) != SDL_GetAtomicInt(&write_idx)) {
            int r = SDL_GetAtomicInt(&read_idx);
            logToFile(&log_buf[r % LOG_CAP]);

            SDL_SetAtomicInt(&read_idx, r + 1);
        }

        SDL_UnlockMutex(log_mutex);
    }
    return 0;
} 


void debugLogInit() {
    #ifdef DEBUG_MODE
        log_running = true;
        debug_output = &log_buf[0]; 
        cpu_log = fopen("cpu_log.csv", "w");
        if (!cpu_log) {
            perror("Failed to open cpu_log.csv");
        }
            fprintf(cpu_log, "PC, SP, PCMEM, AF, BC, DE, HL, PPU_MODE, NS_TAKEN, NS_OVER, NS_CPU, NS_PPU\n");
        SDL_SetAtomicInt(&write_idx, 0);
        SDL_SetAtomicInt(&read_idx, 0);

        log_mutex = SDL_CreateMutex();
        log_cond = SDL_CreateCondition();

        log_thread = SDL_CreateThread(logger_thread, "logger", NULL);
    #endif
}


void debugLog() {
    #ifdef DEBUG_MODE
        if (!cpu_log) return;
        int w = SDL_GetAtomicInt(&write_idx);
        debug_output = &log_buf[w % LOG_CAP];
        SDL_SetAtomicInt(&write_idx, w + 1);
        SDL_SignalCondition(log_cond);
        
    #endif
}


void debugCleanup() {
    #ifdef DEBUG_MODE
    log_running = false;
    SDL_SignalCondition(log_cond);
    SDL_WaitThread(log_thread, NULL);

    if (cpu_log) {
        fclose(cpu_log);
        cpu_log = NULL;
    }

    SDL_DestroyCondition(log_cond);
    SDL_DestroyMutex(log_mutex);
    #endif
}