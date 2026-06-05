#if defined(_WIN32)

#include "compatebility_dumb_controls_internal.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static ft_bool dumb_controls_win32_virtual_key_is_down(int32_t virtual_key)
{
    SHORT key_state;

    key_state = GetAsyncKeyState(virtual_key);
    if ((key_state & 0x8000) != 0)
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control)
{
    if (control == FT_DUMB_CONTROL_UP)
    {
        return (dumb_controls_win32_virtual_key_is_down('W')
            || dumb_controls_win32_virtual_key_is_down(VK_UP));
    }
    if (control == FT_DUMB_CONTROL_DOWN)
    {
        return (dumb_controls_win32_virtual_key_is_down('S')
            || dumb_controls_win32_virtual_key_is_down(VK_DOWN));
    }
    if (control == FT_DUMB_CONTROL_LEFT)
    {
        return (dumb_controls_win32_virtual_key_is_down('A')
            || dumb_controls_win32_virtual_key_is_down(VK_LEFT));
    }
    if (control == FT_DUMB_CONTROL_RIGHT)
    {
        return (dumb_controls_win32_virtual_key_is_down('D')
            || dumb_controls_win32_virtual_key_is_down(VK_RIGHT));
    }
    if (control == FT_DUMB_CONTROL_CONFIRM)
    {
        return (dumb_controls_win32_virtual_key_is_down(VK_RETURN));
    }
    if (control == FT_DUMB_CONTROL_BACK)
    {
        return (dumb_controls_win32_virtual_key_is_down(VK_ESCAPE));
    }
    if (control == FT_DUMB_CONTROL_JUMP)
    {
        return (dumb_controls_win32_virtual_key_is_down(VK_SPACE));
    }
    if (control == FT_DUMB_CONTROL_BOOST)
    {
        return (dumb_controls_win32_virtual_key_is_down('B'));
    }
    if (control == FT_DUMB_CONTROL_MOUSE_PRIMARY)
    {
        return (dumb_controls_win32_virtual_key_is_down(VK_LBUTTON));
    }
    if (control == FT_DUMB_CONTROL_MOUSE_SECONDARY)
    {
        return (dumb_controls_win32_virtual_key_is_down(VK_RBUTTON));
    }
    if (control == FT_DUMB_CONTROL_MOUSE_TERTIARY)
    {
        return (dumb_controls_win32_virtual_key_is_down(VK_MBUTTON));
    }
    return (FT_FALSE);
}

ft_dumb_mouse_delta ft_dumb_platform_mouse_delta(void)
{
    static ft_bool initialized = FT_FALSE;
    static POINT previous_position;
    ft_dumb_mouse_delta delta;
    POINT position;

    delta.x = 0;
    delta.y = 0;
    if (GetCursorPos(&position) == 0)
    {
        initialized = FT_FALSE;
        return (delta);
    }
    if (initialized == FT_TRUE)
    {
        delta.x = position.x - previous_position.x;
        delta.y = position.y - previous_position.y;
    }
    previous_position = position;
    initialized = FT_TRUE;
    return (delta);
}

#endif
