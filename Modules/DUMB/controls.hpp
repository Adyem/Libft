#pragma once

#include "../Basic/basic.hpp"

enum ft_dumb_control
{
    FT_DUMB_CONTROL_UP = 0,
    FT_DUMB_CONTROL_DOWN = 1,
    FT_DUMB_CONTROL_LEFT = 2,
    FT_DUMB_CONTROL_RIGHT = 3,
    FT_DUMB_CONTROL_CONFIRM = 4,
    FT_DUMB_CONTROL_BACK = 5,
    FT_DUMB_CONTROL_JUMP = 6,
    FT_DUMB_CONTROL_BOOST = 7,
    FT_DUMB_CONTROL_MOUSE_PRIMARY = 8,
    FT_DUMB_CONTROL_MOUSE_SECONDARY = 9,
    FT_DUMB_CONTROL_MOUSE_TERTIARY = 10,
    FT_DUMB_CONTROL_COUNT = 11
};

enum ft_dumb_keyboard_layout
{
    FT_DUMB_KEYBOARD_LAYOUT_QWERTY = 0,
    FT_DUMB_KEYBOARD_LAYOUT_AZERTY = 1
};

struct ft_dumb_mouse_delta
{
    int32_t x;
    int32_t y;
};

void    ft_dumb_controls_poll(void);
ft_bool ft_dumb_control_is_down(ft_dumb_control control);
ft_bool ft_dumb_control_was_pressed(ft_dumb_control control);
ft_dumb_mouse_delta ft_dumb_controls_mouse_delta(void);
void    ft_dumb_controls_set_keyboard_layout(ft_dumb_keyboard_layout layout);
void    ft_dumb_controls_set_mouse_captured(ft_bool captured);
ft_bool ft_dumb_controls_get_mouse_captured(void);
ft_dumb_keyboard_layout ft_dumb_controls_get_keyboard_layout(void);
