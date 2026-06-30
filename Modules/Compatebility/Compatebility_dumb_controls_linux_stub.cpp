#if !defined(_WIN32) && !defined(__APPLE__)

#include "compatebility_dumb_controls_internal.hpp"
#include "../Basic/class_nullptr.hpp"

void ft_dumb_controls_linux_register_window(void *display_pointer,
    unsigned long window_id)
{
    (void)display_pointer;
    (void)window_id;
    return ;
}

void ft_dumb_controls_linux_unregister_window(unsigned long window_id)
{
    (void)window_id;
    return ;
}

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control)
{
    (void)control;
    return (FT_FALSE);
}

ft_dumb_mouse_delta ft_dumb_platform_mouse_delta(void)
{
    ft_dumb_mouse_delta delta;

    delta.x = 0;
    delta.y = 0;
    return (delta);
}

#endif
