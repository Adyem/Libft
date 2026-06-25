#if !defined(_WIN32) && !defined(__APPLE__)

#include "compatebility_dumb_controls_internal.hpp"

#include "../Basic/class_nullptr.hpp"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cmath>
#include <cstring>
#include <cstdio>

static Display *g_dumb_controls_linux_display = ft_nullptr;
static Window g_dumb_controls_linux_window = 0;
static ft_bool g_dumb_controls_linux_has_previous_mouse_position = FT_FALSE;
static int32_t g_dumb_controls_linux_previous_mouse_x = 0;
static int32_t g_dumb_controls_linux_previous_mouse_y = 0;

void ft_dumb_controls_linux_register_window(void *display_pointer,
    unsigned long window_id)
{
    g_dumb_controls_linux_display = static_cast<Display *>(display_pointer);
    g_dumb_controls_linux_window = window_id;
    g_dumb_controls_linux_has_previous_mouse_position = FT_FALSE;
    g_dumb_controls_linux_previous_mouse_x = 0;
    g_dumb_controls_linux_previous_mouse_y = 0;
}

void ft_dumb_controls_linux_unregister_window(unsigned long window_id)
{
    if (g_dumb_controls_linux_window == window_id)
    {
        g_dumb_controls_linux_window = 0;
        g_dumb_controls_linux_display = ft_nullptr;
        g_dumb_controls_linux_has_previous_mouse_position = FT_FALSE;
        g_dumb_controls_linux_previous_mouse_x = 0;
        g_dumb_controls_linux_previous_mouse_y = 0;
    }
}

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
    if ((mask & button_mask) != 0)
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control)
{
    ft_dumb_keyboard_layout layout;

    layout = ft_dumb_controls_get_keyboard_layout();
    if (control == FT_DUMB_CONTROL_UP)
    {
        if (layout == FT_DUMB_KEYBOARD_LAYOUT_AZERTY)
            return (dumb_controls_linux_key_is_down(XK_z, XK_Up));
        return (dumb_controls_linux_key_is_down(XK_w, XK_Up));
    }
    if (control == FT_DUMB_CONTROL_DOWN)
        return (dumb_controls_linux_key_is_down(XK_s, XK_Down));
    if (control == FT_DUMB_CONTROL_LEFT)
    {
        if (layout == FT_DUMB_KEYBOARD_LAYOUT_AZERTY)
            return (dumb_controls_linux_key_is_down(XK_q, XK_Left));
        return (dumb_controls_linux_key_is_down(XK_a, XK_Left));
    }
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
    if (control == FT_DUMB_CONTROL_SETTINGS)
        return (dumb_controls_linux_key_is_down(XK_m, XK_M));
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
    ft_dumb_mouse_delta delta;
    Window root_window;
    Window child_window;
    int root_x;
    int root_y;
    int window_x;
    int window_y;
    unsigned int mask;
    int32_t current_mouse_x;
    int32_t current_mouse_y;

    delta.x = 0;
    delta.y = 0;
    if (g_dumb_controls_linux_display == ft_nullptr
        || g_dumb_controls_linux_window == 0)
    {
#ifdef DEBUG
        std::fprintf(stderr,
            "[mouse-debug] poll skipped display=%p window=%lu\n",
            static_cast<void *>(g_dumb_controls_linux_display),
            static_cast<unsigned long>(g_dumb_controls_linux_window));
#endif
        return (delta);
    }
    if (XQueryPointer(g_dumb_controls_linux_display,
            DefaultRootWindow(g_dumb_controls_linux_display), &root_window,
            &child_window, &root_x, &root_y, &window_x, &window_y, &mask) == 0)
    {
        g_dumb_controls_linux_has_previous_mouse_position = FT_FALSE;
#ifdef DEBUG
        std::fprintf(stderr, "[mouse-debug] poll failed\n");
#endif
        return (delta);
    }
    current_mouse_x = root_x;
    current_mouse_y = root_y;
    if (g_dumb_controls_linux_has_previous_mouse_position == FT_TRUE)
    {
        delta.x = current_mouse_x - g_dumb_controls_linux_previous_mouse_x;
        delta.y = current_mouse_y - g_dumb_controls_linux_previous_mouse_y;
    }
    if (g_dumb_controls_linux_window != 0
        && ft_dumb_controls_get_mouse_captured() == FT_TRUE)
    {
        XWindowAttributes win_attrs;
        if (XGetWindowAttributes(g_dumb_controls_linux_display,
                g_dumb_controls_linux_window, &win_attrs) != 0)
        {
            int cx = win_attrs.width / 2;
            int cy = win_attrs.height / 2;
            XWarpPointer(g_dumb_controls_linux_display, None,
                g_dumb_controls_linux_window, 0, 0, 0, 0, cx, cy);
            XFlush(g_dumb_controls_linux_display);
            Window child_ret;
            int root_cx, root_cy;
            XTranslateCoordinates(g_dumb_controls_linux_display,
                g_dumb_controls_linux_window,
                DefaultRootWindow(g_dumb_controls_linux_display),
                cx, cy, &root_cx, &root_cy, &child_ret);
            g_dumb_controls_linux_previous_mouse_x = root_cx;
            g_dumb_controls_linux_previous_mouse_y = root_cy;
        }
        else
        {
            g_dumb_controls_linux_previous_mouse_x = current_mouse_x;
            g_dumb_controls_linux_previous_mouse_y = current_mouse_y;
        }
    }
    else
    {
        g_dumb_controls_linux_previous_mouse_x = current_mouse_x;
        g_dumb_controls_linux_previous_mouse_y = current_mouse_y;
    }
    g_dumb_controls_linux_has_previous_mouse_position = FT_TRUE;
    return (delta);
}

#endif
