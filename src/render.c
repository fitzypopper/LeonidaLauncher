#include "render.h"
#include "app_state.h"
#include "assets.h"
#include "sony_resolve.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// XBM font data (same as liborbisXbmFont)
#define LOWER_ASCII_CODE 32
#define UPPER_ASCII_CODE 126
#define FONT_W 16
#define FONT_H 16
#define BITS_IN_BYTE 8
#define CENTER 2

// Font data (same Exo2-Bold 16x16 as liborbisXbmFont)
static const char xbmFont[95][(FONT_W * FONT_H) / BITS_IN_BYTE] = {
    {0x00}, {0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x01,0xC0,0x01,0xC0,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x01,0xC0,0x01,0xC0,0x01,0xC0,0x01,0xC0,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // ... truncated for brevity - full font data would go here
    // For now we'll use a minimal set and add the full data later
};

// ---------------------------------------------------------------
// Color helpers
// ---------------------------------------------------------------
static inline uint32_t ARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static inline uint32_t lerp_color(uint32_t a, uint32_t b, float t) {
    uint8_t ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
    uint8_t br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
    uint8_t r = ar + (int)((br - ar) * t);
    uint8_t g = ag + (int)((bg - ag) * t);
    uint8_t bl = ab + (int)((bb - ab) * t);
    return ARGB(0xFF, r, g, bl);
}

// ---------------------------------------------------------------
// Framebuffer state (extern for assets.c)
// ---------------------------------------------------------------
VideoState g_video = {0};

// ---------------------------------------------------------------
// Resolution probe (same as liborbis version)
// ---------------------------------------------------------------
static bool probe_video_size(int w, int h)
{
    int handle = sceVideoOutOpen(0xff, 0, 0, NULL);
    if (handle < 0) return false;

    const size_t align = 2 * 1024 * 1024;
    const size_t buf = ((size_t)w * h * 4 + align - 1) & ~(align - 1);
    const size_t total = buf * 2;

    off_t start = 0;
    if (sceKernelAllocateDirectMemory(0, sceKernelGetDirectMemorySize(), total, align, 3, &start) != 0) {
        sceVideoOutClose(handle);
        return false;
    }

    void *base = NULL;
    if (sceKernelMapDirectMemory(&base, total, 0x33, 0, start, align) != 0) {
        sceKernelReleaseDirectMemory(start, total);
        sceVideoOutClose(handle);
        return false;
    }

    void *addr[2] = { base, (char *)base + buf };

    SceVideoOutBufferAttribute attr;
    sceVideoOutSetBufferAttribute(&attr, 0x80000000, 1, 0, (uint32_t)w, (uint32_t)h, (uint32_t)w);

    int ret = sceVideoOutRegisterBuffers(handle, 0, addr, 2, &attr);

    if (ret == 0)
        sceVideoOutUnregisterBuffers(handle, 0);

    sceKernelReleaseDirectMemory(start, total);
    sceVideoOutClose(handle);
    return ret == 0;
}

// ---------------------------------------------------------------
bool render_init(AppState *app, int pad_handle)
{
    (void)pad_handle;

    static const struct { int w, h; } ladder[] = {
        { RES_4K_W,    RES_4K_H    },
        { RES_1080P_W, RES_1080P_H },
        { RES_720P_W,  RES_720P_H  },
    };

    int w = RES_720P_W, h = RES_720P_H;
    for (size_t i = 0; i < sizeof(ladder)/sizeof(ladder[0]); i++) {
        if (probe_video_size(ladder[i].w, ladder[i].h)) {
            w = ladder[i].w; h = ladder[i].h;
            break;
        }
    }

    // Open video out
    g_video.handle = sceVideoOutOpen(0xff, 0, 0, NULL);
    if (g_video.handle < 0) return false;

    // Create flip queue
    if (sceKernelCreateEqueue(&g_video.flip_queue, "flip queue") < 0) return false;
    if (sceVideoOutAddFlipEvent(g_video.flip_queue, g_video.handle, NULL) < 0) return false;

    // Allocate video memory
    const size_t align = 2 * 1024 * 1024;
    const size_t buf = ((size_t)w * h * 4 + align - 1) & ~(align - 1);
    const size_t total = buf * 2;

    off_t start = 0;
    if (sceKernelAllocateDirectMemory(0, sceKernelGetDirectMemorySize(), total, align, 3, &start) != 0) return false;

    void *base = NULL;
    if (sceKernelMapDirectMemory(&base, total, 0x33, 0, start, align) != 0) {
        sceKernelReleaseDirectMemory(start, total);
        return false;
    }

    g_video.base = base;
    g_video.size = total;
    g_video.width = w;
    g_video.height = h;
    g_video.pitch = w;
    g_video.buffers[0] = base;
    g_video.buffers[1] = (char *)base + buf;

    SceVideoOutBufferAttribute attr;
    sceVideoOutSetBufferAttribute(&attr, 0x80000000, 1, 0, w, h, w);

    if (sceVideoOutRegisterBuffers(g_video.handle, 0, g_video.buffers, 2, &attr) != 0) return false;

    if (sceVideoOutSetFlipRate(g_video.handle, 0) != 0) return false;

    // Initial clear
    for (int i = 0; i < 2; i++) {
        memset(g_video.buffers[i], 0, buf);
        sceVideoOutSubmitFlip(g_video.handle, i, 1, -1);
        SceVideoOutFlipStatus status;
        sceVideoOutGetFlipStatus(g_video.handle, &status);
    }

    g_video.initialized = true;
    g_video.current_buffer = 0;

    app->screen_w = w;
    app->screen_h = h;
    app->ui_scale = (float)h / UI_BASE_H;
    app->video_handle = g_video.handle;
    app->flip_queue = g_video.flip_queue;
    app->framebuffer_base = g_video.base;
    app->framebuffer_size = g_video.size;
    app->fb_width = w;
    app->fb_height = h;
    app->fb_pitch = w;

    printf("[RENDER] initialized %dx%d (ui_scale=%.2f)\n", w, h, app->ui_scale);
    return true;
}

void render_finish(void)
{
    if (!g_video.initialized) return;

    for (int i = 0; i < 2; i++) {
        if (g_video.buffers[i]) {
            // buffers are in the mapped region, no separate free
        }
    }
    if (g_video.base) {
        sceKernelReleaseDirectMemory(0, g_video.size);
        g_video.base = NULL;
    }
    if (g_video.handle >= 0) {
        sceVideoOutClose(g_video.handle);
        g_video.handle = -1;
    }
    if (g_video.flip_queue >= 0) {
        // close equeue? not exposed
    }
    g_video.initialized = false;
}

// ---------------------------------------------------------------
void render_begin_frame(AppState *app)
{
    (void)app;
    g_video.current_buffer ^= 1;
    memset(g_video.buffers[g_video.current_buffer], 0, ((size_t)g_video.width * g_video.height * 4));
}

void render_end_frame(AppState *app)
{
    (void)app;
    sceVideoOutSubmitFlip(g_video.handle, g_video.current_buffer, 1, -1);
    SceVideoOutFlipStatus status;
    sceVideoOutGetFlipStatus(g_video.handle, &status);
}

void render_clear(uint32_t argb)
{
    // Not used
    (void)argb;
}

// ---------------------------------------------------------------
// Software rendering into the framebuffer
// ---------------------------------------------------------------
static inline void draw_pixel(int x, int y, uint32_t argb)
{
    if (x < 0 || x >= (int)g_video.width || y < 0 || y >= (int)g_video.height) return;
    uint32_t *fb = (uint32_t *)g_video.buffers[g_video.current_buffer];
    fb[y * g_video.pitch + x] = argb;
}

void render_rect(AppState *app, int x, int y, int w, int h, uint32_t argb)
{
    (void)app;
    for (int dy = 0; dy < h; dy++) {
        int yy = y + dy;
        if (yy < 0 || yy >= (int)g_video.height) continue;
        uint32_t *row = (uint32_t *)g_video.buffers[g_video.current_buffer] + yy * g_video.pitch;
        int x_start = x < 0 ? 0 : x;
        int x_end = (x + w > (int)g_video.width) ? g_video.width : x + w;
        for (int xx = x_start; xx < x_end; xx++) {
            row[xx] = argb;
        }
    }
}

// ---------------------------------------------------------------
// Text rendering from XBM font
// ---------------------------------------------------------------
void render_text(AppState *app, int x, int y, const char *str, uint32_t color, uint32_t color_b)
{
    (void)app;
    int scale = (int)app->ui_scale;
    if (scale < 1) scale = 1;

    for (const char *p = str; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (c < LOWER_ASCII_CODE || c > UPPER_ASCII_CODE) {
            x += FONT_W * scale;
            continue;
        }

        const uint8_t *bit = (const uint8_t *)xbmFont[c - LOWER_ASCII_CODE];
        int bytes_per_row = FONT_W / BITS_IN_BYTE;

        for (int i = 0; i < (FONT_W * FONT_H) / BITS_IN_BYTE; i++) {
            int row = i / bytes_per_row;
            int col_byte = i % bytes_per_row;
            uint8_t byte = bit[i];

            for (int b = 0; b < BITS_IN_BYTE; b++) {
                if (!(byte & (1 << b))) continue;

                int px = x + col_byte * BITS_IN_BYTE + b;
                int py = y + row;

                uint32_t col = lerp_color(color, color_b, (float)row / (FONT_H - 1));

                for (int sy = 0; sy < scale; sy++)
                    for (int sx = 0; sx < scale; sx++)
                        draw_pixel(px * scale + sx, py * scale + sy, col);
            }
        }
        x += FONT_W * scale;
    }
}

void render_text_centered(AppState *app, int y, const char *str, uint32_t color, uint32_t color_b)
{
    int n = 0; while (str[n]) n++;
    int scale = (int)app->ui_scale;
    if (scale < 1) scale = 1;
    int width = n * FONT_W * scale;
    render_text(app, (app->screen_w - width) / 2, y, str, color, color_b);
}

int render_text_width(const char *str)
{
    int n = 0; if (!str) return 0; while (str[n]) n++; return n * FONT_W;
}

// ---------------------------------------------------------------
// GTA VI logo
// ---------------------------------------------------------------
void render_gta6_logo(AppState *app, int cx, int y)
{
    int scale = (int)app->ui_scale;
    if (scale < 1) scale = 1;

    const char *line1 = "GRAND THEFT AUTO";
    int n1 = 0; while (line1[n1]) n1++;
    int w1 = n1 * FONT_W * scale;
    render_text(app, cx - w1 / 2, y, line1, C_NEON_PINK, C_NEON_ORANGE);

    const char *vi = "VI";
    int big = scale * 3;
    int w2 = 2 * FONT_W * big;
    render_text(app, cx - w2 / 2, y + FONT_H * scale * 2, vi, C_GOLD, C_NEON_ORANGE);
    // outline
    render_text(app, cx - w2 / 2 - 2, y + FONT_H * scale * 2, vi, C_NEON_MAGENTA, C_NEON_VIOLET);
    render_text(app, cx - w2 / 2, y + FONT_H * scale * 2, vi, C_GOLD, C_NEON_ORANGE);
}