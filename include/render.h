#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <stdbool.h>

#include "app_state.h"

typedef struct AppState AppState;

// Framebuffer state (for assets.c access)
typedef struct VideoState {
    int handle;
    int flip_queue;
    void *base;
    size_t size;
    uint32_t width, height, pitch;
    uint32_t current_buffer;
    void *buffers[2];
    bool initialized;
} VideoState;

extern VideoState g_video;

bool render_init(AppState *app, int pad_handle);
void render_finish(void);
void render_begin_frame(AppState *app);
void render_end_frame(AppState *app);
void render_clear(uint32_t argb);
void render_rect(AppState *app, int x, int y, int w, int h, uint32_t argb);
void render_text(AppState *app, int x, int y, const char *str, uint32_t color, uint32_t color_b);
void render_text_centered(AppState *app, int y, const char *str, uint32_t color, uint32_t color_b);
int  render_text_width(const char *str);
void render_gta6_logo(AppState *app, int cx, int y);

#endif