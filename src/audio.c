#include "audio.h"
#include "sony_resolve.h"
#include "menu.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define SAMPLE_RATE 48000
#define AUDIO_BUF_SAMPLES 1024
#define AUDIO_BUF_SIZE (AUDIO_BUF_SAMPLES * 2 * 2)  // stereo * 2 bytes * samples
#define TWO_PI 6.28318530718f

#define ORBISAUDIO_VOLUME_MAX 32768

// Audio port/channel (PS5 uses port 0, index 0 for main)
#define AUDIO_PORT_TYPE 0
#define AUDIO_PORT_INDEX 0

typedef struct {
    bool active;
    int type;
    float phase;
    float t;
    float dur;
    float freq_a, freq_b;
    float gain;
} Voice;

static Voice g_voices[4];
static int g_music_vol = 22000;
static int g_sfx_vol = 26000;
static float g_music_time = 0.0f;

static const struct {
    float freq_a, freq_b, dur, gain;
    bool square;
    bool dual;
} sfx_def[SFX_COUNT] = {
    [SFX_NAVIGATE] = { 240.0f, 420.0f, 0.055f, 0.55f, false, false },
    [SFX_SELECT]   = { 440.0f, 880.0f, 0.110f, 0.70f, false, true  },
    [SFX_BACK]     = { 420.0f, 240.0f, 0.055f, 0.55f, false, false },
    [SFX_ERROR]    = { 150.0f, 150.0f, 0.220f, 0.60f, true,  false },
};

static float music_sample(float t)
{
    float f1 = 110.0f;
    float f2 = 164.81f;
    float f3 = 220.0f;
    float drift = 1.0f + 0.004f * sinf(t * 0.23f);
    float pad = 0.0f;
    pad += sinf(TWO_PI * f1 * drift * t) * 0.45f;
    pad += sinf(TWO_PI * f2 * drift * t + 0.7f) * 0.30f;
    pad += sinf(TWO_PI * f3 * t + 1.3f) * 0.22f;
    pad *= 0.55f + 0.45f * sinf(t * 0.35f);
    float beat = fmodf(t, 0.6f);
    float pulse = expf(-beat * 9.0f) * 0.35f * sinf(TWO_PI * 55.0f * t);
    return pad * 0.55f + pulse;
}

static void audio_fill_buffer(void *buf, int samples)
{
    int16_t *dst = (int16_t *)buf;
    float music_g = (g_music_vol / (float)ORBISAUDIO_VOLUME_MAX) * 1.0f;
    float sfx_g   = (g_sfx_vol / (float)ORBISAUDIO_VOLUME_MAX) * 1.0f;

    for (int i = 0; i < samples; i++) {
        float out = 0.0f;
        out += music_sample(g_music_time) * music_g * 0.6f;

        for (int v = 0; v < 4; v++) {
            Voice *vc = &g_voices[v];
            if (!vc->active) continue;
            float prog = vc->t / vc->dur;
            if (prog >= 1.0f) { vc->active = false; continue; }
            float freq = vc->freq_a + (vc->freq_b - vc->freq_a) * prog;
            float env = expf(-prog * 5.0f);
            float s;
            if (sfx_def[vc->type].square) {
                s = (sinf(TWO_PI * freq * vc->t) >= 0) ? 1.0f : -1.0f;
            } else if (sfx_def[vc->type].dual) {
                s = 0.6f * sinf(TWO_PI * freq * vc->t) + 0.4f * sinf(TWO_PI * freq * 2.0f * vc->t);
            } else {
                s = sinf(TWO_PI * freq * vc->t);
            }
            out += s * env * vc->gain * sfx_g;
            vc->phase += freq / SAMPLE_RATE;
            vc->t += 1.0f / SAMPLE_RATE;
        }

        if (out > 1.0f) out = 1.0f;
        if (out < -1.0f) out = -1.0f;
        int16_t sample = (int16_t)(out * 32000.0f);
        *dst++ = sample;  // left
        *dst++ = sample;  // right
        g_music_time += 1.0f / SAMPLE_RATE;
    }
}

static int16_t g_audio_buffer[AUDIO_BUF_SAMPLES * 2];

void audio_init(AppState *app)
{
    (void)app;
    if (sceAudioOutInit() != 0) return;
    if (sceAudioOutOpen(AUDIO_PORT_TYPE, AUDIO_PORT_INDEX, 2, SAMPLE_RATE, 16, AUDIO_BUF_SIZE, NULL) != 0) return;
    app->music_volume = g_music_vol;
    app->sfx_volume = g_sfx_vol;
    printf("[AUDIO] initialized: %d Hz, %d samples\n", SAMPLE_RATE, AUDIO_BUF_SAMPLES);
}

void audio_finish(void)
{
    sceAudioOutClose(AUDIO_PORT_TYPE, AUDIO_PORT_INDEX);
}

void audio_play_sfx(AppState *app, int sfx_id)
{
    (void)app;
    if (sfx_id < 0 || sfx_id >= SFX_COUNT) return;

    int slot = -1;
    float oldest = 0;
    for (int v = 0; v < 4; v++) {
        if (!g_voices[v].active) { slot = v; break; }
        if (g_voices[v].t > oldest) { oldest = g_voices[v].t; slot = v; }
    }
    if (slot < 0) slot = 0;

    Voice *vc = &g_voices[slot];
    vc->active = true;
    vc->type = sfx_id;
    vc->phase = 0;
    vc->t = 0;
    vc->dur = sfx_def[sfx_id].dur;
    vc->freq_a = sfx_def[sfx_id].freq_a;
    vc->freq_b = sfx_def[sfx_id].freq_b;
    vc->gain = sfx_def[sfx_id].gain;
}

void audio_update_music(AppState *app)
{
    (void)app;
    // Fill and submit audio buffer
    audio_fill_buffer(g_audio_buffer, AUDIO_BUF_SAMPLES);
    sceAudioOutOutput(AUDIO_PORT_TYPE, AUDIO_PORT_INDEX, g_audio_buffer, AUDIO_BUF_SAMPLES * 2 * 2);
}

void audio_set_music_volume(AppState *app, int vol)
{
    if (vol < 0) vol = 0;
    if (vol > ORBISAUDIO_VOLUME_MAX) vol = ORBISAUDIO_VOLUME_MAX;
    g_music_vol = vol;
    app->music_volume = vol;
}

void audio_set_sfx_volume(AppState *app, int vol)
{
    if (vol < 0) vol = 0;
    if (vol > ORBISAUDIO_VOLUME_MAX) vol = ORBISAUDIO_VOLUME_MAX;
    g_sfx_vol = vol;
    app->sfx_volume = vol;
}