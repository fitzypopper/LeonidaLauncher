/*
 * GTA VI fake menu — PS5 payload (PS5SDK)
 * Entry point: payload_main(struct payload_args *args)
 */

#include <ps5/payload_main.h>
#include <ps5/libkernel.h>

// Define socklen_t before libc.h (PS5SDK uses __socklen_t)
typedef __socklen_t socklen_t;
#include <ps5/libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

// Our headers
#include "app_state.h"
#include "menu.h"
#include "render.h"
#include "input.h"
#include "audio.h"
#include "assets.h"
#include "sony_resolve.h"

// ---------------------------------------------------------------
// Loading tips
// ---------------------------------------------------------------
static const char *loading_tips[] = {
    "ENTERING LEONIDA, GOOD LUCK OUT THERE",
    "VICE CITY, WHERE THE NEON NEVER SLEEPS",
    "TRUST NO ONE, ESPECIALLY YOUR PARTNER",
    "THE AMERICAN DREAM IS ALIVE IN LEONIDA",
    "FAST CARS, FAST MONEY, FAST LANE",
    "WELCOME TO THE SUNSHINE STATE",
    "LUCIA AND JASON, PARTNERS IN CRIME",
    "EVERY HEIST HAS A PRICE",
};
#define TIP_COUNT (sizeof(loading_tips) / sizeof(loading_tips[0]))

static void start_loading(AppState *app, const char *tip)
{
    app->show_loading = true;
    app->loading_progress = 0.0f;
    app->loading_start_ms = sceKernelGetProcessTime() / 1000;
    strncpy(app->loading_tip, tip, sizeof(app->loading_tip) - 1);
}

// ---------------------------------------------------------------
// Global app state
// ---------------------------------------------------------------
static AppState g_app;

// ---------------------------------------------------------------
// Menu actions (forward declarations)
// ---------------------------------------------------------------
void action_settings(void *userdata);
void action_extras(void *userdata);
void action_settings_audio(void *userdata);
void action_settings_video(void *userdata);
void action_settings_controls(void *userdata);
void action_extras_gallery(void *userdata);
void action_extras_credits(void *userdata);
void action_extras_legal(void *userdata);
void action_resume(void *userdata);
void action_story_mode(void *userdata);
void action_online(void *userdata);
void action_quit(void *userdata);
void menu_open_pause(AppState *app);

// ---------------------------------------------------------------
// Menu actions
// ---------------------------------------------------------------
void action_story_mode(void *userdata) { start_loading((AppState *)userdata, loading_tips[(int)(g_app.frame % TIP_COUNT)]); }
void action_online(void *userdata)     { start_loading((AppState *)userdata, "CONNECTING TO GRAND THEFT AUTO ONLINE"); }
void action_quit(void *userdata)       { g_app.running = false; }

void action_settings(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = malloc(sizeof(Menu));
    if (!m) return;
    menu_init(m, "SETTINGS");
    m->parent = app->current_menu;
    menu_add_item(m, "AUDIO", MENU_ITEM_SUBMENU, action_settings_audio, app);
    menu_add_item(m, "VIDEO", MENU_ITEM_SUBMENU, action_settings_video, app);
    menu_add_item(m, "CONTROLS", MENU_ITEM_SUBMENU, action_settings_controls, app);
    menu_add_item(m, "BACK", MENU_ITEM_ACTION, action_resume, app);
    app->current_menu = m;
}

void action_extras(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = malloc(sizeof(Menu));
    if (!m) return;
    menu_init(m, "EXTRAS");
    m->parent = app->current_menu;
    menu_add_item(m, "ARTWORK GALLERY", MENU_ITEM_SUBMENU, action_extras_gallery, app);
    menu_add_item(m, "CREDITS", MENU_ITEM_SUBMENU, action_extras_credits, app);
    menu_add_item(m, "LEGAL", MENU_ITEM_SUBMENU, action_extras_legal, app);
    menu_add_item(m, "BACK", MENU_ITEM_ACTION, action_resume, app);
    app->current_menu = m;
}

void action_resume(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = app->current_menu;
    if (m && m->parent) {
        Menu *parent = m->parent;
        menu_destroy(m);
        app->current_menu = parent;
    }
}

void action_settings_audio(void *userdata) {
    AppState *app = (AppState *)userdata;
    if (app->music_volume > 22000) {
        audio_set_music_volume(app, 12000);
        audio_set_sfx_volume(app, 16000);
    } else {
        audio_set_music_volume(app, 26000);
        audio_set_sfx_volume(app, 30000);
    }
    audio_play_sfx(app, SFX_SELECT);
}

void action_settings_video(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = malloc(sizeof(Menu));
    if (!m) return;
    menu_init(m, "VIDEO");
    m->parent = app->current_menu;
    char line[64];
    snprintf(line, sizeof(line), "RESOLUTION %dX%d", app->screen_w, app->screen_h);
    menu_add_item(m, line, MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "BACK", MENU_ITEM_ACTION, action_resume, app);
    app->current_menu = m;
}

void action_settings_controls(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = malloc(sizeof(Menu));
    if (!m) return;
    menu_init(m, "CONTROLS");
    m->parent = app->current_menu;
    menu_add_item(m, "D PAD NAVIGATE", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "CROSS SELECT", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "CIRCLE BACK", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "OPTIONS PAUSE", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "BACK", MENU_ITEM_ACTION, action_resume, app);
    app->current_menu = m;
}

void action_extras_gallery(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = malloc(sizeof(Menu));
    if (!m) return;
    menu_init(m, "GALLERY");
    m->parent = app->current_menu;
    menu_add_item(m, "NO ARTWORK YET", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "BACK", MENU_ITEM_ACTION, action_resume, app);
    app->current_menu = m;
}

void action_extras_credits(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = malloc(sizeof(Menu));
    if (!m) return;
    menu_init(m, "CREDITS");
    m->parent = app->current_menu;
    menu_add_item(m, "PS5SDK", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "RELAPSE EXPLOIT", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "BACK", MENU_ITEM_ACTION, action_resume, app);
    app->current_menu = m;
}

void action_extras_legal(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = malloc(sizeof(Menu));
    if (!m) return;
    menu_init(m, "LEGAL");
    m->parent = app->current_menu;
    menu_add_item(m, "FAN PROJECT", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "NOT AFFILIATED", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "WITH ROCKSTAR", MENU_ITEM_NONE, NULL, app);
    menu_add_item(m, "BACK", MENU_ITEM_ACTION, action_resume, app);
    app->current_menu = m;
}

static void create_main_menu(AppState *app)
{
    Menu *m = malloc(sizeof(Menu));
    if (!m) return;

    menu_init(m, "MAIN MENU");
    menu_add_item(m, "STORY MODE", MENU_ITEM_ACTION, action_story_mode, app);
    menu_add_item(m, "GTA ONLINE", MENU_ITEM_ACTION, action_online, app);
    menu_add_item(m, "SETTINGS", MENU_ITEM_SUBMENU, action_settings, app);
    menu_add_item(m, "EXTRAS", MENU_ITEM_SUBMENU, action_extras, app);
    menu_add_item(m, "QUIT", MENU_ITEM_ACTION, action_quit, app);

    app->current_menu = m;
}



// ---------------------------------------------------------------
// Loading screen rendering
// ---------------------------------------------------------------
static void render_loading(AppState *app)
{
    int s = (int)app->ui_scale;
    if (s < 1) s = 1;
    int fh = 16 * s;

    render_gta6_logo(app, app->screen_w / 2, app->screen_h / 5);

    int bar_w = app->screen_w * 3 / 5;
    int bar_h = 6 * s;
    int bar_x = (app->screen_w - bar_w) / 2;
    int bar_y = app->screen_h * 7 / 10;
    render_rect(app, bar_x - 2 * s, bar_y - 2 * s, bar_w + 4 * s, bar_h + 4 * s, 0xFF606060);
    render_rect(app, bar_x, bar_y, (int)(bar_w * app->loading_progress), bar_h, 0xFFFFD700);

    render_text_centered(app, app->screen_h * 78 / 100, app->loading_tip, 0xFFB4B4B4, 0xFF606060);
    render_text_centered(app, app->screen_h - fh * 2,
                         "FAN PROJECT, NOT AFFILIATED WITH ROCKSTAR GAMES",
                         0xFF606060, 0xFF606060);
}

// ---------------------------------------------------------------
// Frame loop
// ---------------------------------------------------------------
static void update(AppState *app)
{
    input_update(app);

    uint64_t now_ms = sceKernelGetProcessTime() / 1000;
    uint64_t dt = now_ms - app->last_ms;
    if (dt > 100) dt = 100;
    app->last_ms = now_ms;
    app->frame++;

    if (app->show_loading) {
        uint64_t elapsed = now_ms - app->loading_start_ms;
        app->loading_progress = (float)elapsed / 2500.0f;
        if (app->loading_progress >= 1.0f) {
            app->loading_progress = 1.0f;
            app->show_loading = false;
        }
        if (input_pressed_circle()) {
            app->show_loading = false;
            audio_play_sfx(app, SFX_BACK);
        }
        return;
    }

    if (!app->current_menu)
        return;

    menu_update(app->current_menu, app);

    if (input_pressed_options()) {
        if (!app->current_menu->parent)
            menu_open_pause(app);
        else
            action_resume(app);
        audio_play_sfx(app, SFX_SELECT);
        input_haptic(120, 120, 0);
    }

    audio_update_music(app);
}

static void draw(AppState *app)
{
    render_begin_frame(app);
    assets_draw_skybox(app, app->frame);

    if (app->show_loading) {
        render_loading(app);
    } else if (app->current_menu) {
        menu_render(app->current_menu, app);
    }

    render_end_frame(app);
}

// ---------------------------------------------------------------
// payload_main
// ---------------------------------------------------------------
int payload_main(struct payload_args *args)
{
    (void)args;  // CRT already initialized libkernel/libc via dlsym

    // Resolve Sony libraries
    if (sony_resolve_all() < 0) {
        return -1;
    }

    // Initialize subsystems
    sceSystemServiceHideSplashScreen();
    sceUserServiceInitialize();
    int user_id = -1;
    sceUserServiceGetInitialUser(&user_id);
    if (user_id < 0) user_id = 0;

    scePadInit();
    int pad_handle = scePadOpen(user_id, 0, 0, NULL);
    if (pad_handle < 0) pad_handle = 0;

    sceAudioOutInit();

    // Initialize app state
    memset(&g_app, 0, sizeof(AppState));
    g_app.running = true;
    g_app.music_volume = 22000;
    g_app.sfx_volume = 26000;

    // Initialize our modules
    if (!render_init(&g_app, pad_handle)) return -1;
    audio_init(&g_app);
    assets_init(&g_app);
    input_init(pad_handle);
    create_main_menu(&g_app);

    // Frame loop
    uint64_t next = sceKernelGetProcessTime() + 16666;
    while (g_app.running) {
        update(&g_app);
        draw(&g_app);

        uint64_t now = sceKernelGetProcessTime();
        if (now < next)
            sceKernelUsleep((unsigned int)(next - now));
        next += 16666;
        if (now > next)
            next = now + 16666;
    }

    // Cleanup
    assets_finish();
    audio_finish();
    render_finish();
    sony_resolve_cleanup();

    return 0;
}