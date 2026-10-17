#ifndef ASSETS_H
#define ASSETS_H

#include <stdint.h>
#include <stdbool.h>

#include "app_state.h"

typedef struct AppState AppState;

void assets_init(AppState *app);
void assets_finish(void);
void assets_build_skybox(AppState *app);
void assets_draw_skybox(AppState *app, uint64_t frame);
uint32_t assets_neon_color(int index, float t);

#endif