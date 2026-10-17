#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>
#include <stdbool.h>

// size_t for -nostdinc environments (64-bit = unsigned long long)
#ifndef size_t
typedef unsigned long long size_t;
#endif

// Forward declarations
typedef struct Menu Menu;
typedef struct AppState AppState;

// Video output types (SceKernelEqueue is int in this SDK)
typedef int SceKernelEqueue;

// Full app state definition
typedef struct AppState {
    Menu *current_menu;
    bool running;

    uint64_t frame;
    uint64_t last_ms;

    bool show_loading;
    float loading_progress;
    char loading_tip[128];
    uint64_t loading_start_ms;

    int screen_w;
    int screen_h;
    float ui_scale;

    int music_volume;
    int sfx_volume;

    // Video handles
    int video_handle;
    int flip_queue;
    void *framebuffer_base;
    size_t framebuffer_size;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
} AppState;

#endif