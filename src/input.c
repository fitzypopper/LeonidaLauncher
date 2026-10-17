#include "input.h"
#include "sony_resolve.h"
#include "menu.h"

#include <stdio.h>

// PS5 Pad button masks
#define ORBISPAD_UP        0x00000010
#define ORBISPAD_DOWN      0x00000040
#define ORBISPAD_LEFT      0x00000080
#define ORBISPAD_RIGHT     0x00000020
#define ORBISPAD_L1        0x00000400
#define ORBISPAD_R1        0x00000800
#define ORBISPAD_TRIANGLE  0x00001000
#define ORBISPAD_CIRCLE    0x00002000
#define ORBISPAD_CROSS     0x00004000
#define ORBISPAD_SQUARE    0x00008000
#define ORBISPAD_OPTIONS   0x00000008

typedef struct ScePadData {
    unsigned int buttons;
    uint8_t lx, ly, rx, ry;
    uint8_t l2, r2;
    uint16_t padding;
    // ... rest not needed
} ScePadData;

static int g_pad_handle = -1;
static ScePadData g_pad_data_curr = {0};
static ScePadData g_pad_data_last = {0};
static unsigned int g_buttons_pressed = 0;
static unsigned int g_buttons_released = 0;

void input_init(int pad_handle)
{
    g_pad_handle = pad_handle;
    g_buttons_pressed = 0;
    g_buttons_released = 0;

    // Set lightbar to GTA VI deep pink
    ScePadLightBarParam lb = {255, 20, 147, 0};
    scePadSetLightBar(g_pad_handle, &lb);
}

void input_update(AppState *app)
{
    (void)app;

    g_pad_data_last = g_pad_data_curr;
    scePadRead(g_pad_handle, &g_pad_data_curr, 1);

    unsigned int curr = g_pad_data_curr.buttons;
    unsigned int last = g_pad_data_last.buttons;

    g_buttons_pressed  = curr & ~last;
    g_buttons_released = ~curr & last;
}

bool input_pressed_up(void)       { return g_buttons_pressed & ORBISPAD_UP; }
bool input_pressed_down(void)     { return g_buttons_pressed & ORBISPAD_DOWN; }
bool input_pressed_left(void)     { return g_buttons_pressed & ORBISPAD_LEFT; }
bool input_pressed_right(void)    { return g_buttons_pressed & ORBISPAD_RIGHT; }
bool input_pressed_cross(void)    { return g_buttons_pressed & ORBISPAD_CROSS; }
bool input_pressed_circle(void)   { return g_buttons_pressed & ORBISPAD_CIRCLE; }
bool input_pressed_options(void)  { return g_buttons_pressed & ORBISPAD_OPTIONS; }
bool input_pressed_l1(void)       { return g_buttons_pressed & ORBISPAD_L1; }
bool input_pressed_r1(void)       { return g_buttons_pressed & ORBISPAD_R1; }

void input_haptic(uint8_t left, uint8_t right, uint8_t dur_ms)
{
    (void)dur_ms;
    ScePadVibrationParam param = {0};
    if (left)  { param.valid |= 0x01; param.motorLeft = left; }
    if (right) { param.valid |= 0x02; param.motorRight = right; }
    scePadSetVibration(g_pad_handle, &param);
}

void input_set_lightbar(uint8_t r, uint8_t g, uint8_t b)
{
    ScePadLightBarParam lb = {r, g, b, 0};
    scePadSetLightBar(g_pad_handle, &lb);
}