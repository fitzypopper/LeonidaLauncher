#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

#include "app_state.h"

#define MAX_MENU_ITEMS 8
#define MENU_ANIM_MS   150
#define VISIBLE_ITEMS  6

#define UI_BASE_W 1920.0f
#define UI_BASE_H 1080.0f

#define RES_4K_W     3840
#define RES_4K_H     2160
#define RES_1080P_W  1920
#define RES_1080P_H  1080
#define RES_720P_W   1280
#define RES_720P_H   720

// Font constants (shared with render.c)
#define FONT_W 16
#define FONT_H 16

// Colors (ARGB)
#define C_NEON_PINK   0xFFFF1493
#define C_NEON_MAGENTA 0xFFFF00FF
#define C_NEON_VIOLET 0xFF8A2BE2
#define C_NEON_BLUE   0xFF00BFFF
#define C_NEON_YELLOW 0xFFFFFF00
#define C_NEON_ORANGE 0xFFFFA500
#define C_GOLD        0xFFFFD700
#define C_WHITE       0xFFFFFFFF
#define C_BLACK       0xFF000000
#define C_PANEL       0xC814141C
#define C_GRAY_TEXT   0xFFB4B4B4
#define C_GRAY_DIM    0xFF606060

typedef enum {
    MENU_ITEM_ACTION,
    MENU_ITEM_SUBMENU,
    MENU_ITEM_NONE
} MenuItemType;

typedef struct MenuItem {
    char text[64];
    MenuItemType type;
    void (*action)(void *userdata);
    void *userdata;
    bool enabled;
    float selected;
} MenuItem;

typedef struct Menu {
    char title[64];
    MenuItem items[MAX_MENU_ITEMS];
    int item_count;
    int selected_index;
    int scroll_offset;
    struct Menu *parent;
} Menu;

// Forward declarations for action functions
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

void menu_init(Menu *menu, const char *title);
void menu_add_item(Menu *menu, const char *text, MenuItemType type,
                   void (*action)(void *), void *userdata);
void menu_update(Menu *menu, AppState *app);
void menu_render(Menu *menu, AppState *app);
void menu_destroy(Menu *menu);

#endif