#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>

#include "app_state.h"

enum { SFX_NAVIGATE = 0, SFX_SELECT, SFX_BACK, SFX_ERROR, SFX_COUNT };

void audio_init(AppState *app);
void audio_finish(void);
void audio_play_sfx(AppState *app, int sfx_id);
void audio_update_music(AppState *app);
void audio_set_music_volume(AppState *app, int vol);
void audio_set_sfx_volume(AppState *app, int vol);

#endif