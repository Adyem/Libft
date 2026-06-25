#if defined(_WIN32)

#include "compatebility_dumb_controls_internal.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HWND g_dumb_controls_win32_capture_window = nullptr;
static ft_bool g_dumb_controls_win32_cursor_hidden = FT_FALSE;
static ft_bool g_dumb_controls_win32_has_previous_mouse_position = FT_FALSE;
static POINT g_dumb_controls_win32_previous_mouse_position;

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

static void dumb_controls_win32_show_cursor(ft_bool show)
{
    if (show == FT_TRUE)
    {
        while (ShowCursor(TRUE) < 0)
        {
        }
        g_dumb_controls_win32_cursor_hidden = FT_FALSE;
        return ;
    }
    while (ShowCursor(FALSE) >= 0)
    {
    }
    g_dumb_controls_win32_cursor_hidden = FT_TRUE;
}

void ft_dumb_controls_win32_register_capture_window(void *window_handle)
{
    RECT client_rect;
    POINT top_left;
    POINT bottom_right;
    RECT screen_rect;

    g_dumb_controls_win32_capture_window = static_cast<HWND>(window_handle);
    g_dumb_controls_win32_has_previous_mouse_position = FT_FALSE;
    if (g_dumb_controls_win32_capture_window == nullptr)
        return ;
    GetClientRect(g_dumb_controls_win32_capture_window, &client_rect);
    top_left.x = client_rect.left;
    top_left.y = client_rect.top;
    bottom_right.x = client_rect.right;
    bottom_right.y = client_rect.bottom;
    ClientToScreen(g_dumb_controls_win32_capture_window, &top_left);
    ClientToScreen(g_dumb_controls_win32_capture_window, &bottom_right);
    screen_rect.left = top_left.x;
    screen_rect.top = top_left.y;
    screen_rect.right = bottom_right.x;
    screen_rect.bottom = bottom_right.y;
    ClipCursor(&screen_rect);
    SetCapture(g_dumb_controls_win32_capture_window);
    dumb_controls_win32_show_cursor(FT_FALSE);
}

void ft_dumb_controls_win32_unregister_capture_window(void)
{
    if (g_dumb_controls_win32_capture_window != nullptr)
    {
        ClipCursor(nullptr);
        ReleaseCapture();
    }
    if (g_dumb_controls_win32_cursor_hidden == FT_TRUE)
        dumb_controls_win32_show_cursor(FT_TRUE);
    g_dumb_controls_win32_capture_window = nullptr;
    g_dumb_controls_win32_has_previous_mouse_position = FT_FALSE;
}

ft_bool ft_dumb_platform_control_is_down(ft_dumb_control control)
{
    ft_dumb_keyboard_layout layout;

    layout = ft_dumb_controls_get_keyboard_layout();
    if (control == FT_DUMB_CONTROL_UP)
    {
        if (layout == FT_DUMB_KEYBOARD_LAYOUT_AZERTY)
        {
            return (dumb_controls_win32_virtual_key_is_down('Z')
                || dumb_controls_win32_virtual_key_is_down(VK_UP));
        }
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
        if (layout == FT_DUMB_KEYBOARD_LAYOUT_AZERTY)
        {
            return (dumb_controls_win32_virtual_key_is_down('Q')
                || dumb_controls_win32_virtual_key_is_down(VK_LEFT));
        }
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
    if (control == FT_DUMB_CONTROL_SETTINGS)
    {
        return (dumb_controls_win32_virtual_key_is_down('M'));
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
    ft_dumb_mouse_delta delta;
    POINT position;
    RECT client_rect;
    POINT center;
    POINT top_left;
    POINT bottom_right;

    delta.x = 0;
    delta.y = 0;
    if (GetCursorPos(&position) == 0)
    {
        g_dumb_controls_win32_has_previous_mouse_position = FT_FALSE;
        return (delta);
    }
    if (g_dumb_controls_win32_capture_window != nullptr)
    {
        if (GetForegroundWindow() != g_dumb_controls_win32_capture_window)
        {
            g_dumb_controls_win32_has_previous_mouse_position = FT_FALSE;
            return (delta);
        }
        GetClientRect(g_dumb_controls_win32_capture_window, &client_rect);
        center.x = (client_rect.left + client_rect.right) / 2;
        center.y = (client_rect.top + client_rect.bottom) / 2;
        top_left = center;
        bottom_right = center;
        ClientToScreen(g_dumb_controls_win32_capture_window, &top_left);
        ClientToScreen(g_dumb_controls_win32_capture_window, &bottom_right);
        center.x = top_left.x;
        center.y = top_left.y;
        if (g_dumb_controls_win32_has_previous_mouse_position == FT_TRUE)
        {
            delta.x = position.x - g_dumb_controls_win32_previous_mouse_position.x;
            delta.y = position.y - g_dumb_controls_win32_previous_mouse_position.y;
        }
        SetCursorPos(center.x, center.y);
        g_dumb_controls_win32_previous_mouse_position = center;
        g_dumb_controls_win32_has_previous_mouse_position = FT_TRUE;
        return (delta);
    }
    if (g_dumb_controls_win32_has_previous_mouse_position == FT_TRUE)
    {
        delta.x = position.x - g_dumb_controls_win32_previous_mouse_position.x;
        delta.y = position.y - g_dumb_controls_win32_previous_mouse_position.y;
    }
    g_dumb_controls_win32_previous_mouse_position = position;
    g_dumb_controls_win32_has_previous_mouse_position = FT_TRUE;
    return (delta);
}

#endif
