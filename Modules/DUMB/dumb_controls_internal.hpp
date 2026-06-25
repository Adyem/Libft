#pragma once

#include "controls.hpp"

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control);
ft_dumb_mouse_delta ft_dumb_platform_mouse_delta(void);

#ifdef LIBFT_TEST_BUILD
typedef ft_bool (*ft_dumb_test_platform_control_is_down_fn)(ft_dumb_control control);
typedef ft_dumb_mouse_delta (*ft_dumb_test_platform_mouse_delta_fn)(void);

void ft_dumb_controls_set_test_platform_control_is_down(
    ft_dumb_test_platform_control_is_down_fn control_provider);
void ft_dumb_controls_set_test_platform_mouse_delta(
    ft_dumb_test_platform_mouse_delta_fn mouse_delta_provider);
void ft_dumb_controls_clear_test_platform_hooks(void);
#endif
