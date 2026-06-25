#pragma once

#include "../DUMB/controls.hpp"

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control);
ft_dumb_mouse_delta ft_dumb_platform_mouse_delta(void);
void ft_dumb_controls_set_keyboard_layout(ft_dumb_keyboard_layout layout);
ft_dumb_keyboard_layout ft_dumb_controls_get_keyboard_layout(void);
void ft_dumb_controls_win32_register_capture_window(void *window_handle);
void ft_dumb_controls_win32_unregister_capture_window(void);
void ft_dumb_controls_linux_register_window(void *display_pointer,
    unsigned long window_id);
void ft_dumb_controls_linux_unregister_window(unsigned long window_id);
