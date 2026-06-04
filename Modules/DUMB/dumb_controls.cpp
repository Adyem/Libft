#include "dumb_controls_internal.hpp"

static ft_bool g_dumb_control_down[FT_DUMB_CONTROL_COUNT];
static ft_bool g_dumb_control_pressed[FT_DUMB_CONTROL_COUNT];
static ft_bool g_dumb_control_previous_down[FT_DUMB_CONTROL_COUNT];
static ft_dumb_mouse_delta g_dumb_mouse_delta;

void ft_dumb_controls_poll(void)
{
    uint32_t index_control;
    ft_bool  current_down;

    index_control = 0U;
    while (index_control < FT_DUMB_CONTROL_COUNT)
    {
        current_down = ft_dumb_platform_control_is_down(
            static_cast<ft_dumb_control>(index_control));
        g_dumb_control_down[index_control] = current_down;
        g_dumb_control_pressed[index_control] = FT_FALSE;
        if (current_down == FT_TRUE
            && g_dumb_control_previous_down[index_control] == FT_FALSE)
        {
            g_dumb_control_pressed[index_control] = FT_TRUE;
        }
        g_dumb_control_previous_down[index_control] = current_down;
        index_control = index_control + 1U;
    }
    g_dumb_mouse_delta = ft_dumb_platform_mouse_delta();
    return ;
}

ft_bool ft_dumb_control_is_down(ft_dumb_control control)
{
    if (control < FT_DUMB_CONTROL_UP || control >= FT_DUMB_CONTROL_COUNT)
    {
        return (FT_FALSE);
    }
    return (g_dumb_control_down[control]);
}

ft_bool ft_dumb_control_was_pressed(ft_dumb_control control)
{
    if (control < FT_DUMB_CONTROL_UP || control >= FT_DUMB_CONTROL_COUNT)
    {
        return (FT_FALSE);
    }
    return (g_dumb_control_pressed[control]);
}

ft_dumb_mouse_delta ft_dumb_controls_mouse_delta(void)
{
    return (g_dumb_mouse_delta);
}
