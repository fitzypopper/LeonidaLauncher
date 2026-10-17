#include "menu.h"
#include "render.h"
#include "input.h"
#include "audio.h"
#include "assets.h"

#include <ps5/libkernel.h>

// Define socklen_t before libc.h (PS5SDK uses __socklen_t)
typedef __socklen_t socklen_t;
#include <ps5/libc.h>
#include <stdio.h>
#include <string.h>

static inline float animate_toward(float cur, float target, float speed)
{
    if (cur < target) { cur += speed; return cur > target ? target : cur; }
    if (cur > target) { cur -= speed; return cur < target ? target : cur; }
    return cur;
}

void menu_init(Menu *menu, const char *title)
{
    memset(menu, 0, sizeof(Menu));
    strncpy(menu->title, title, sizeof(menu->title) - 1);
    menu->selected_index = 0;
    menu->parent = NULL;
}

void menu_add_item(Menu *menu, const char *text, MenuItemType type,
                   void (*action)(void *), void *userdata)
{
    if (menu->item_count >= MAX_MENU_ITEMS) return;
    MenuItem *it = &menu->items[menu->item_count++];
    memset(it, 0, sizeof(MenuItem));
    strncpy(it->text, text, sizeof(it->text) - 1);
    it->type = type;
    it->action = action;
    it->userdata = userdata;
    it->enabled = true;
}

void menu_destroy(Menu *menu)
{
    if (menu) free(menu);
}

static void menu_navigate(Menu *menu, int dir, AppState *app)
{
    int i = menu->selected_index;
    for (int n = 0; n < menu->item_count; n++) {
        i += dir;
        if (i < 0) i = menu->item_count - 1;
        if (i >= menu->item_count) i = 0;
        if (menu->items[i].enabled) {
            menu->selected_index = i;
            break;
        }
    }
    input_haptic(70, 70, 0);
    audio_play_sfx(app, SFX_NAVIGATE);
}

void menu_update(Menu *menu, AppState *app)
{
    float speed = 0.12f;
    for (int i = 0; i < menu->item_count; i++) {
        MenuItem *it = &menu->items[i];
        it->selected = animate_toward(it->selected, i == menu->selected_index ? 1.0f : 0.0f, speed);
    }

    if (menu->selected_index < menu->scroll_offset)
        menu->scroll_offset = menu->selected_index;
    if (menu->selected_index >= menu->scroll_offset + VISIBLE_ITEMS)
        menu->scroll_offset = menu->selected_index - VISIBLE_ITEMS + 1;

    if (input_pressed_up())   menu_navigate(menu, -1, app);
    if (input_pressed_down()) menu_navigate(menu, +1, app);

    if (input_pressed_l1() || input_pressed_r1()) {
        input_haptic(60, 60, 0);
        audio_play_sfx(app, SFX_NAVIGATE);
    }

    if (input_pressed_cross()) {
        MenuItem *it = &menu->items[menu->selected_index];
        input_haptic(160, 160, 0);
        audio_play_sfx(app, SFX_SELECT);
        if (it->enabled && it->action) it->action(it->userdata);
    } else if (input_pressed_circle()) {
        input_haptic(90, 90, 0);
        audio_play_sfx(app, SFX_BACK);
        if (menu->parent) {
            Menu *parent = menu->parent;
            menu_destroy(menu);
            app->current_menu = parent;
        }
    }
}

void menu_render(Menu *menu, AppState *app)
{
    int s = (int)app->ui_scale;
    if (s < 1) s = 1;

    int screen_w = app->screen_w;
    int screen_h = app->screen_h;
    int fh = FONT_H * s;

    int panel_w = screen_w / 3;
    int panel_x = screen_w / 16;

    render_rect(app, panel_x, screen_h / 8, panel_w, screen_h * 3 / 4, C_PANEL);
    render_rect(app, panel_x, screen_h / 8, panel_w, 2 * s, C_GOLD);
    render_rect(app, panel_x, screen_h / 8 + screen_h * 3 / 4 - 2 * s, panel_w, 2 * s, C_GOLD);

    int title_y = screen_h / 8 + fh;
    render_text(app, panel_x + fh / 2, title_y, menu->title, C_GOLD, C_NEON_ORANGE);

    int y = title_y + fh * 2;
    for (int i = 0; i < VISIBLE_ITEMS; i++) {
        int idx = menu->scroll_offset + i;
        if (idx >= menu->item_count) break;

        MenuItem *it = &menu->items[idx];
        int item_y = y + i * (int)(fh * 1.6f);
        uint32_t fg = it->enabled ? C_WHITE : C_GRAY_DIM;

        if (it->selected > 0.01f) {
            uint8_t a = (uint8_t)(60 + 90 * it->selected);
            uint32_t hl = (a << 24) | 0xFFD700;
            render_rect(app, panel_x + fh / 2 - s, item_y - s, panel_w - fh, (int)(fh * 1.3f), hl);
            render_rect(app, panel_x + fh / 2 - 4 * s, item_y, 3 * s, fh, C_GOLD);
            fg = C_GOLD;
        }

        render_text(app, panel_x + fh, item_y, it->text, fg, fg);
        if (it->type == MENU_ITEM_SUBMENU)
            render_text(app, panel_x + panel_w - fh, item_y, ">", fg, fg);
    }

    if (menu->scroll_offset > 0)
        render_text_centered(app, title_y + fh + fh / 2, "^", C_GRAY_TEXT, C_GRAY_TEXT);
    if (menu->scroll_offset + VISIBLE_ITEMS < menu->item_count)
        render_text_centered(app, screen_h / 8 + screen_h * 3 / 4 - fh, "v", C_GRAY_TEXT, C_GRAY_TEXT);

    float t = (float)(app->frame % 3000) / 3000.0f;
    int logo_x = panel_x + panel_w + (screen_w - panel_x - panel_w) / 2;
    render_gta6_logo(app, logo_x, screen_h / 6);

    uint32_t neon = assets_neon_color((int)(app->frame / 60) % 6, t);
    const char *tag = "VICE CITY";
    int n = 0; while (tag[n]) n++;
    render_text(app, logo_x - (n * FONT_W * s) / 2, screen_h / 2, tag, neon, C_NEON_ORANGE);

    const char *hint = "CROSS SELECT     CIRCLE BACK";
    int hn = 0; while (hint[hn]) hn++;
    render_text(app, (screen_w - hn * FONT_W * s) / 2, screen_h - fh * 2, hint, C_GRAY_TEXT, C_GRAY_DIM);
}

static void pause_action_resume(void *userdata) {
    AppState *app = (AppState *)userdata;
    Menu *m = app->current_menu;
    if (m && m->parent) {
        Menu *parent = m->parent;
        menu_destroy(m);
        app->current_menu = parent;
    }
}

static void pause_action_quit(void *userdata) {
    AppState *app = (AppState *)userdata;
    app->running = false;
}

void menu_open_pause(AppState *app)
{
    Menu *pause = malloc(sizeof(Menu));
    if (!pause) return;
    menu_init(pause, "PAUSED");
    pause->parent = app->current_menu;
    menu_add_item(pause, "RESUME", MENU_ITEM_ACTION, pause_action_resume, app);
    menu_add_item(pause, "SETTINGS", MENU_ITEM_SUBMENU, action_settings, app);
    menu_add_item(pause, "EXTRAS", MENU_ITEM_SUBMENU, action_extras, app);
    menu_add_item(pause, "QUIT TO TITLE", MENU_ITEM_ACTION, pause_action_quit, app);
    app->current_menu = pause;
}