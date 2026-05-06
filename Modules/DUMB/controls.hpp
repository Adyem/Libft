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
    FT_DUMB_CONTROL_COUNT = 6
};

void    ft_dumb_controls_poll(void);
ft_bool ft_dumb_control_is_down(ft_dumb_control control);
ft_bool ft_dumb_control_was_pressed(ft_dumb_control control);
