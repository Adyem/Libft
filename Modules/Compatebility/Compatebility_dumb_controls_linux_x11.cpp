#if !defined(_WIN32) && !defined(__APPLE__)

#include "compatebility_dumb_controls_internal.hpp"

#include "../Basic/class_nullptr.hpp"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cstring>

static Display *g_dumb_controls_linux_display = ft_nullptr;
static Window g_dumb_controls_linux_window = 0;

void ft_dumb_controls_linux_register_window(void *display_pointer,
    unsigned long window_id)
{
    g_dumb_controls_linux_display = static_cast<Display *>(display_pointer);
    g_dumb_controls_linux_window = static_cast<Window>(window_id);
}

void ft_dumb_controls_linux_unregister_window(unsigned long window_id)
{
    if (g_dumb_controls_linux_window == static_cast<Window>(window_id))
    {
        g_dumb_controls_linux_window = 0;
        g_dumb_controls_linux_display = ft_nullptr;
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

static Window dumb_controls_linux_pointer_window(Display *display_pointer)
{
    Window root_window;
    Window child_window;
    int root_x;
    int root_y;
    int window_x;
    int window_y;
    unsigned int mask;

    if (XQueryPointer(display_pointer, DefaultRootWindow(display_pointer),
            &root_window, &child_window, &root_x, &root_y, &window_x,
            &window_y, &mask) == 0)
        return (0);
    if (child_window != 0)
        return (child_window);
    return (root_window);
}

static Window dumb_controls_linux_target_window(Display *display_pointer)
{
    if (g_dumb_controls_linux_window != 0)
        return (g_dumb_controls_linux_window);
    return (dumb_controls_linux_pointer_window(display_pointer));
}

static ft_bool dumb_controls_linux_query_local_pointer(Display *display_pointer,
    Window target_window, int *pointer_x, int *pointer_y, int *center_x,
    int *center_y)
{
    XWindowAttributes attributes;
    Window root_window;
    Window child_window;
    int root_x;
    int root_y;
    unsigned int mask;

    if (target_window == 0)
        return (FT_FALSE);
    if (XGetWindowAttributes(display_pointer, target_window, &attributes) == 0)
        return (FT_FALSE);
    if (attributes.width <= 0 || attributes.height <= 0)
        return (FT_FALSE);
    if (XQueryPointer(display_pointer, target_window, &root_window,
            &child_window, &root_x, &root_y, pointer_x, pointer_y,
            &mask) == 0)
        return (FT_FALSE);
    *center_x = attributes.width / 2;
    *center_y = attributes.height / 2;
    return (FT_TRUE);
}

ft_dumb_mouse_delta ft_dumb_platform_mouse_delta(void)
{
    static ft_bool initialized = FT_FALSE;
    static Window grabbed_window = 0;
    ft_dumb_mouse_delta delta;
    Display *display_pointer;
    Window target_window;
    int center_x;
    int center_y;
    int pointer_x;
    int pointer_y;

    delta.x = 0;
    delta.y = 0;
    display_pointer = g_dumb_controls_linux_display;
    if (display_pointer == ft_nullptr)
        display_pointer = dumb_controls_linux_open_display();
    if (display_pointer == ft_nullptr)
        return (delta);
    target_window = dumb_controls_linux_target_window(display_pointer);
    if (target_window == 0)
    {
        initialized = FT_FALSE;
        return (delta);
    }
    if (target_window != grabbed_window)
    {
        grabbed_window = 0;
        initialized = FT_FALSE;
    }
    if (XGrabPointer(display_pointer, target_window, False,
            PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, target_window, None, CurrentTime)
        == GrabSuccess)
        grabbed_window = target_window;
    if (dumb_controls_linux_query_local_pointer(display_pointer, target_window,
            &pointer_x, &pointer_y, &center_x, &center_y) == FT_TRUE)
    {
        if (initialized == FT_TRUE)
        {
            delta.x = pointer_x - center_x;
            delta.y = pointer_y - center_y;
        }
    }
    else
    {
        XWindowAttributes attributes;

        initialized = FT_FALSE;
        center_x = DisplayWidth(display_pointer, DefaultScreen(display_pointer)) / 2;
        center_y = DisplayHeight(display_pointer, DefaultScreen(display_pointer)) / 2;
        if (XGetWindowAttributes(display_pointer, target_window, &attributes) != 0
            && attributes.width > 0 && attributes.height > 0)
        {
            center_x = attributes.width / 2;
            center_y = attributes.height / 2;
        }
    }
    XWarpPointer(display_pointer, None, target_window, 0, 0, 0, 0,
        center_x, center_y);
    XSync(display_pointer, False);
    initialized = FT_TRUE;
    return (delta);
}

#endif
