#include "assets.h"
#include "render.h"
#include "menu.h"

#include <math.h>
#include <stdio.h>

#define SKYBOX_W 512
#define SKYBOX_H 512

static uint32_t *g_skybox = NULL;

static const uint32_t neon_stops[6] = {
    0xFFFF1493, 0xFFFF00FF, 0xFF8A2BE2,
    0xFF00BFFF, 0xFFFFFF00, 0xFFFFA500,
};

static inline float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline float hash_noise(int x, int y)
{
    uint32_t n = (uint32_t)(x * 374761393 + y * 668265263);
    n = (n ^ (n >> 13)) * 1274126177u;
    return ((n & 0xFFFF) / 65535.0f) * 2.0f - 1.0f;
}

static inline uint32_t rgb_to_argb(float r, float g, float b)
{
    uint32_t R = (uint32_t)(clampf(r, 0.0f, 1.0f) * 255.0f);
    uint32_t G = (uint32_t)(clampf(g, 0.0f, 1.0f) * 255.0f);
    uint32_t B = (uint32_t)(clampf(b, 0.0f, 1.0f) * 255.0f);
    return 0xFF000000u | (R << 16) | (G << 8) | B;
}

static inline void argb_to_rgb(uint32_t c, float *r, float *g, float *b)
{
    *r = ((c >> 16) & 0xFF) / 255.0f;
    *g = ((c >> 8) & 0xFF) / 255.0f;
    *b = (c & 0xFF) / 255.0f;
}

void assets_build_skybox(AppState *app)
{
    (void)app;
    g_skybox = (uint32_t *)malloc(SKYBOX_W * SKYBOX_H * 4);
    if (!g_skybox) return;

    for (int y = 0; y < SKYBOX_H; y++) {
        float v = (float)y / (float)(SKYBOX_H - 1);
        float seg = v * 5.0f;
        int i = (int)seg; if (i > 4) i = 4;
        float f = seg - (float)i;
        f = f * f * (3.0f - 2.0f * f);

        float r0, g0, b0, r1, g1, b1;
        argb_to_rgb(neon_stops[i],     &r0, &g0, &b0);
        argb_to_rgb(neon_stops[i + 1], &r1, &g1, &b1);

        float r = r0 + (r1 - r0) * f;
        float g = g0 + (g1 - g0) * f;
        float b = b0 + (b1 - b0) * f;

        float glow = expf(-(v - 0.62f) * (v - 0.62f) * 40.0f);
        r += glow * 0.25f;
        b += glow * 0.10f;

        for (int x = 0; x < SKYBOX_W; x++) {
            float n = hash_noise(x, y) * 0.015f;
            float rr = r + n, gg = g + n, bb = b + n;

            if (y > SKYBOX_H * 0.74f) {
                float trunk = sinf(x * 0.06f + 3.0f) * 8.0f;
                float base_x = SKYBOX_W * 0.5f + trunk;
                float dx = fabsf(x - base_x) - 2.0f;
                float dy = y - SKYBOX_H * 0.74f;

                bool frond = false;
                for (int k = -3; k <= 3; k++) {
                    float slope = k * 0.55f;
                    float fx = dx - slope * dy;
                    if (fabsf(fx) < 6.0f && dy > 0 && dy < 110.0f) frond = true;
                }
                bool trunk_hit = fabsf(x - (base_x + sinf(y * 0.04f) * 4.0f)) < 3.0f && y > SKYBOX_H * 0.74f;

                if (frond || trunk_hit) {
                    rr *= 0.06f; gg *= 0.06f; bb *= 0.10f;
                }
            }

            g_skybox[y * SKYBOX_W + x] = rgb_to_argb(rr, gg, bb);
        }
    }
    printf("[ASSETS] skybox generated (%dx%d)\n", SKYBOX_W, SKYBOX_H);
}

void assets_draw_skybox(AppState *app, uint64_t frame)
{
    if (!g_skybox) return;

    int drift = (int)((frame / 4) % (uint64_t)SKYBOX_H);
    int w = app->screen_w;
    int h = app->screen_h + SKYBOX_H;

    for (int x = 0; x < w; x += SKYBOX_W) {
        for (int y = -SKYBOX_H; y < h; y += SKYBOX_H) {
            // Draw skybox tile using software rendering
            for (int ty = 0; ty < SKYBOX_H; ty++) {
                int sy = y + drift + ty;
                if (sy < 0 || sy >= app->screen_h) continue;
                uint32_t *dst = (uint32_t *)g_video.buffers[g_video.current_buffer] + sy * g_video.pitch + x;
                uint32_t *src = g_skybox + ty * SKYBOX_W;
                int copy_w = (x + SKYBOX_W > w) ? w - x : SKYBOX_W;
                for (int tx = 0; tx < copy_w; tx++) {
                    dst[tx] = src[tx];
                }
            }
        }
    }
}

uint32_t assets_neon_color(int index, float t)
{
    float seg = t - (float)(int)t;
    seg *= 5.0f;
    int i = (int)seg % 5;
    float f = seg - (float)i;

    float r0, g0, b0, r1, g1, b1;
    argb_to_rgb(neon_stops[(index + i) % 6],     &r0, &g0, &b0);
    argb_to_rgb(neon_stops[(index + i + 1) % 6], &r1, &g1, &b1);

    return rgb_to_argb(r0 + (r1 - r0) * f, g0 + (g1 - g0) * f, b0 + (b1 - b0) * f);
}

void assets_init(AppState *app)
{
    assets_build_skybox(app);
}

void assets_finish(void)
{
    if (g_skybox) {
        free(g_skybox);
        g_skybox = NULL;
    }
}