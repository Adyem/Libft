#if !defined(_WIN32) && !defined(__APPLE__)

#include "compatebility_dumb_controls_internal.hpp"

#include "../CPP_class/class_nullptr.hpp"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cstring>

static Display *dumb_controls_linux_open_display(void)
{
    static Display *display_pointer = ft_nullptr;

    if (display_pointer == ft_nullptr)
    {
        display_pointer = XOpenDisplay(ft_nullptr);
    }
    return (display_pointer);
}

static ft_bool dumb_controls_linux_key_is_down(KeySym key_symbol_primary,
    KeySym key_symbol_secondary)
{
    Display *display_pointer;
    char    key_states[32];
    KeyCode key_code;
    uint32_t key_index;
    uint32_t bit_index;

    display_pointer = dumb_controls_linux_open_display();
    if (display_pointer == ft_nullptr)
    {
        return (FT_FALSE);
    }
    std::memset(key_states, 0, sizeof(key_states));
    XQueryKeymap(display_pointer, key_states);
    key_code = XKeysymToKeycode(display_pointer, key_symbol_primary);
    key_index = static_cast<uint32_t>(key_code) / 8U;
    bit_index = static_cast<uint32_t>(key_code) % 8U;
    if (key_index < sizeof(key_states)
        && (key_states[key_index] & (1 << bit_index)) != 0)
    {
        return (FT_TRUE);
    }
    if (key_symbol_secondary != NoSymbol)
    {
        key_code = XKeysymToKeycode(display_pointer, key_symbol_secondary);
        key_index = static_cast<uint32_t>(key_code) / 8U;
        bit_index = static_cast<uint32_t>(key_code) % 8U;
        if (key_index < sizeof(key_states)
            && (key_states[key_index] & (1 << bit_index)) != 0)
        {
            return (FT_TRUE);
        }
    }
    return (FT_FALSE);
}

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control)
{
    if (control == FT_DUMB_CONTROL_UP)
    {
        return (dumb_controls_linux_key_is_down(XK_w, XK_Up));
    }
    if (control == FT_DUMB_CONTROL_DOWN)
    {
        return (dumb_controls_linux_key_is_down(XK_s, XK_Down));
    }
    if (control == FT_DUMB_CONTROL_LEFT)
    {
        return (dumb_controls_linux_key_is_down(XK_a, XK_Left));
    }
    if (control == FT_DUMB_CONTROL_RIGHT)
    {
        return (dumb_controls_linux_key_is_down(XK_d, XK_Right));
    }
    if (control == FT_DUMB_CONTROL_CONFIRM)
    {
        return (dumb_controls_linux_key_is_down(XK_Return, XK_space));
    }
    if (control == FT_DUMB_CONTROL_BACK)
    {
        return (dumb_controls_linux_key_is_down(XK_Escape, NoSymbol));
    }
    return (FT_FALSE);
}

#endif
