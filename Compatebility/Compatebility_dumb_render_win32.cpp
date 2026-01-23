#if defined(_WIN32)

#include "../DUMB/dumb_render_internal.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct ft_render_win32_state
{
    HINSTANCE       instance_handle;
    HWND            window_handle;
    HDC             window_device_context;

    HBITMAP         dib_section;
    void            *dib_pixels;

    int             width;
    int             height;

    bool            is_fullscreen;

    DWORD           window_style;
    RECT            windowed_rect;
};

static LRESULT CALLBACK ft_render_win32_wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    ft_render_win32_state *state;

    state = (ft_render_win32_state *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (message == WM_CLOSE)
    {
        DestroyWindow(hwnd);
        return (0);
    }
    if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return (0);
    }
    (void)state;
    return (DefWindowProc(hwnd, message, wparam, lparam));
}

static ft_render_platform_result ft_render_win32_create_dib(
    ft_render_win32_state *state,
    ft_render_framebuffer *out_framebuffer
)
{
    BITMAPINFO      bitmap_info;
    void            *pixels;
    HBITMAP         dib_section;

    ZeroMemory(&bitmap_info, sizeof(bitmap_info));
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = state->width;
    bitmap_info.bmiHeader.biHeight = -state->height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    pixels = NULL;
    dib_section = CreateDIBSection(
        state->window_device_context,
        &bitmap_info,
        DIB_RGB_COLORS,
        &pixels,
        NULL,
        0
    );

    if (dib_section == NULL || pixels == NULL)
    {
        ft_render_platform_result result;

        result.error_code = ft_render_error_platform_failure;
        result.system_error_code = (int)GetLastError();
        return (result);
    }

    state->dib_section = dib_section;
    state->dib_pixels = pixels;

    out_framebuffer->width = state->width;
    out_framebuffer->height = state->height;
    out_framebuffer->pixels = (uint32_t *)pixels;

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_get_primary_screen_size(ft_render_screen_size *out_size)
{
    if (out_size == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    out_size->width = (int)GetSystemMetrics(SM_CXSCREEN);
    out_size->height = (int)GetSystemMetrics(SM_CYSCREEN);

    if (out_size->width <= 0 || out_size->height <= 0)
    {
        return ((ft_render_platform_result){ ft_render_error_platform_failure, (int)GetLastError() });
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_create_window(
    void **out_platform_state,
    ft_render_framebuffer *out_framebuffer,
    const ft_render_window_desc &desc
)
{
    ft_render_win32_state  *state;
    WNDCLASS               window_class;
    DWORD                  style;
    RECT                   rect;
    HWND                   hwnd;

    if (out_platform_state == NULL || out_framebuffer == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = (ft_render_win32_state *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ft_render_win32_state));
    if (state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_out_of_memory, 0 });
    }

    state->instance_handle = GetModuleHandle(NULL);
    state->width = desc.width;
    state->height = desc.height;
    state->is_fullscreen = false;
    state->dib_section = NULL;
    state->dib_pixels = NULL;

    ZeroMemory(&window_class, sizeof(window_class));
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = ft_render_win32_wndproc;
    window_class.hInstance = state->instance_handle;
    window_class.lpszClassName = "ft_render_window_class";

    if (RegisterClass(&window_class) == 0)
    {
        ft_render_platform_result result;

        result.error_code = ft_render_error_platform_failure;
        result.system_error_code = (int)GetLastError();
        HeapFree(GetProcessHeap(), 0, state);
        return (result);
    }

    style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    if ((desc.flags & ft_render_window_flag_resizable) != 0)
    {
        style = style | WS_SIZEBOX | WS_MAXIMIZEBOX;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = desc.width;
    rect.bottom = desc.height;
    AdjustWindowRect(&rect, style, FALSE);

    hwnd = CreateWindowEx(
        0,
        window_class.lpszClassName,
        desc.title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        state->instance_handle,
        NULL
    );

    if (hwnd == NULL)
    {
        ft_render_platform_result result;

        result.error_code = ft_render_error_platform_failure;
        result.system_error_code = (int)GetLastError();
        HeapFree(GetProcessHeap(), 0, state);
        return (result);
    }

    state->window_handle = hwnd;
    state->window_style = style;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    state->window_device_context = GetDC(hwnd);
    if (state->window_device_context == NULL)
    {
        ft_render_platform_result result;

        result.error_code = ft_render_error_platform_failure;
        result.system_error_code = (int)GetLastError();
        DestroyWindow(hwnd);
        HeapFree(GetProcessHeap(), 0, state);
        return (result);
    }

    {
        ft_render_platform_result dib_result;

        dib_result = ft_render_win32_create_dib(state, out_framebuffer);
        if (dib_result.error_code != ft_render_ok)
        {
            ReleaseDC(hwnd, state->window_device_context);
            DestroyWindow(hwnd);
            HeapFree(GetProcessHeap(), 0, state);
            return (dib_result);
        }
    }

    *out_platform_state = state;
    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_destroy_window(
    void **platform_state,
    ft_render_framebuffer *framebuffer
)
{
    ft_render_win32_state  *state;

    if (platform_state == NULL || *platform_state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_ok, 0 });
    }

    state = (ft_render_win32_state *)(*platform_state);

    if (state->dib_section != NULL)
    {
        DeleteObject(state->dib_section);
        state->dib_section = NULL;
        state->dib_pixels = NULL;
    }

    if (state->window_device_context != NULL && state->window_handle != NULL)
    {
        ReleaseDC(state->window_handle, state->window_device_context);
        state->window_device_context = NULL;
    }

    if (state->window_handle != NULL)
    {
        DestroyWindow(state->window_handle);
        state->window_handle = NULL;
    }

    HeapFree(GetProcessHeap(), 0, state);
    *platform_state = NULL;

    if (framebuffer != NULL)
    {
        framebuffer->width = 0;
        framebuffer->height = 0;
        framebuffer->pixels = NULL;
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_poll_events(
    void *platform_state,
    bool *out_should_close
)
{
    MSG   message;
    BOOL  got_message;

    if (platform_state == NULL || out_should_close == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    *out_should_close = false;

    got_message = PeekMessage(&message, NULL, 0, 0, PM_REMOVE);
    while (got_message != 0)
    {
        if (message.message == WM_QUIT)
        {
            *out_should_close = true;
        }
        TranslateMessage(&message);
        DispatchMessage(&message);

        got_message = PeekMessage(&message, NULL, 0, 0, PM_REMOVE);
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_present(
    void *platform_state,
    ft_render_framebuffer *framebuffer
)
{
    ft_render_win32_state  *state;
    HDC                    memory_dc;
    HBITMAP                old_bitmap;

    if (platform_state == NULL || framebuffer == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = (ft_render_win32_state *)platform_state;

    memory_dc = CreateCompatibleDC(state->window_device_context);
    if (memory_dc == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_platform_failure, (int)GetLastError() });
    }

    old_bitmap = (HBITMAP)SelectObject(memory_dc, state->dib_section);
    BitBlt(
        state->window_device_context,
        0,
        0,
        framebuffer->width,
        framebuffer->height,
        memory_dc,
        0,
        0,
        SRCCOPY
    );
    SelectObject(memory_dc, old_bitmap);
    DeleteDC(memory_dc);

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

static void ft_render_win32_enter_fullscreen(ft_render_win32_state *state)
{
    MONITORINFO monitor_info;
    HMONITOR    monitor;

    GetWindowRect(state->window_handle, &state->windowed_rect);

    monitor = MonitorFromWindow(state->window_handle, MONITOR_DEFAULTTONEAREST);
    monitor_info.cbSize = sizeof(monitor_info);
    GetMonitorInfo(monitor, &monitor_info);

    SetWindowLong(state->window_handle, GWL_STYLE, state->window_style & ~WS_OVERLAPPEDWINDOW);

    SetWindowPos(
        state->window_handle,
        HWND_TOP,
        monitor_info.rcMonitor.left,
        monitor_info.rcMonitor.top,
        monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
        monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
        SWP_NOOWNERZORDER | SWP_FRAMECHANGED
    );

    state->is_fullscreen = true;
    return ;
}

static void ft_render_win32_leave_fullscreen(ft_render_win32_state *state)
{
    SetWindowLong(state->window_handle, GWL_STYLE, state->window_style);

    SetWindowPos(
        state->window_handle,
        NULL,
        state->windowed_rect.left,
        state->windowed_rect.top,
        state->windowed_rect.right - state->windowed_rect.left,
        state->windowed_rect.bottom - state->windowed_rect.top,
        SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED
    );

    state->is_fullscreen = false;
    return ;
}

ft_render_platform_result ft_render_platform_set_fullscreen(
    void *platform_state,
    bool enabled
)
{
    ft_render_win32_state  *state;

    if (platform_state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = (ft_render_win32_state *)platform_state;

    if (enabled == true && state->is_fullscreen == false)
    {
        ft_render_win32_enter_fullscreen(state);
    }
    else if (enabled == false && state->is_fullscreen == true)
    {
        ft_render_win32_leave_fullscreen(state);
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

#endif
