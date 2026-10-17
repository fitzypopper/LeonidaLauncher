#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>

#include "app_state.h"

void input_init(int pad_handle);
void input_update(AppState *app);

bool input_pressed_up(void);
bool input_pressed_down(void);
bool input_pressed_left(void);
bool input_pressed_right(void);
bool input_pressed_cross(void);
bool input_pressed_circle(void);
bool input_pressed_options(void);
bool input_pressed_l1(void);
bool input_pressed_r1(void);

void input_haptic(uint8_t left, uint8_t right, uint8_t dur_ms);
void input_set_lightbar(uint8_t r, uint8_t g, uint8_t b);

#endif