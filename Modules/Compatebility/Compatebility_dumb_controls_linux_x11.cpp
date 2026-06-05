#if !defined(_WIN32) && !defined(__APPLE__)

#include "compatebility_dumb_controls_internal.hpp"

#include "../Basic/class_nullptr.hpp"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cstring>

static Display *dumb_controls_linux_open_display(void)
{
    static Display *display_pointer = ft_nullptr;

    if (display_pointer == ft_nullptr)
        display_pointer = XOpenDisplay(ft_nullptr);
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
        return (FT_FALSE);
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
            return (FT_TRUE);
    }
    return (FT_FALSE);
}

static ft_bool dumb_controls_linux_mouse_button_is_down(unsigned int button_mask)
{
    Display *display_pointer;
    Window root_window;
    Window child_window;
    int root_x;
    int root_y;
    int window_x;
    int window_y;
    unsigned int mask;

    display_pointer = dumb_controls_linux_open_display();
    if (display_pointer == ft_nullptr)
        return (FT_FALSE);
    if (XQueryPointer(display_pointer, DefaultRootWindow(display_pointer),
            &root_window, &child_window, &root_x, &root_y, &window_x,
            &window_y, &mask) == 0)
        return (FT_FALSE);
    return ((mask & button_mask) != 0 ? FT_TRUE : FT_FALSE);
}

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control)
{
    if (control == FT_DUMB_CONTROL_UP)
        return (dumb_controls_linux_key_is_down(XK_w, XK_Up));
    if (control == FT_DUMB_CONTROL_DOWN)
        return (dumb_controls_linux_key_is_down(XK_s, XK_Down));
    if (control == FT_DUMB_CONTROL_LEFT)
        return (dumb_controls_linux_key_is_down(XK_a, XK_Left));
    if (control == FT_DUMB_CONTROL_RIGHT)
        return (dumb_controls_linux_key_is_down(XK_d, XK_Right));
    if (control == FT_DUMB_CONTROL_CONFIRM)
        return (dumb_controls_linux_key_is_down(XK_Return, NoSymbol));
    if (control == FT_DUMB_CONTROL_BACK)
        return (dumb_controls_linux_key_is_down(XK_Escape, NoSymbol));
    if (control == FT_DUMB_CONTROL_JUMP)
        return (dumb_controls_linux_key_is_down(XK_space, NoSymbol));
    if (control == FT_DUMB_CONTROL_BOOST)
        return (dumb_controls_linux_key_is_down(XK_b, XK_B));
    if (control == FT_DUMB_CONTROL_MOUSE_PRIMARY)
        return (dumb_controls_linux_mouse_button_is_down(Button1Mask));
    if (control == FT_DUMB_CONTROL_MOUSE_SECONDARY)
        return (dumb_controls_linux_mouse_button_is_down(Button3Mask));
    if (control == FT_DUMB_CONTROL_MOUSE_TERTIARY)
        return (dumb_controls_linux_mouse_button_is_down(Button2Mask));
    return (FT_FALSE);
}

ft_dumb_mouse_delta ft_dumb_platform_mouse_delta(void)
{
    static ft_bool initialized = FT_FALSE;
    ft_dumb_mouse_delta delta;
    Display *display_pointer;
    Window root_window;
    Window child_window;
    int center_x;
    int center_y;
    int root_x;
    int root_y;
    int window_x;
    int window_y;
    unsigned int mask;

    delta.x = 0;
    delta.y = 0;
    display_pointer = dumb_controls_linux_open_display();
    if (display_pointer == ft_nullptr)
        return (delta);
    center_x = DisplayWidth(display_pointer, DefaultScreen(display_pointer)) / 2;
    center_y = DisplayHeight(display_pointer, DefaultScreen(display_pointer)) / 2;
    if (XQueryPointer(display_pointer,
            DefaultRootWindow(display_pointer), &root_window, &child_window,
            &root_x, &root_y, &window_x, &window_y, &mask) == 0)
    {
        initialized = FT_FALSE;
        return (delta);
    }
    if (initialized == FT_TRUE)
    {
        delta.x = root_x - center_x;
        delta.y = root_y - center_y;
    }
    XWarpPointer(display_pointer, None, DefaultRootWindow(display_pointer),
        0, 0, 0, 0, center_x, center_y);
    XFlush(display_pointer);
    initialized = FT_TRUE;
    return (delta);
}

#endif
