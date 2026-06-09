#if defined(_WIN32)

#include "GpuWindow_windows.hpp"

# define WIN32_LEAN_AND_MEAN
#endif



class GpuWindowWindows : public GpuWindow
{
    HWND _hwnd;
    HDC _dc;
    HGLRC _ctx;
    bool _close_flag;
    int _width;
    int _height;

    public:
        GpuWindowWindows() noexcept
            : _hwnd(nullptr), _dc(nullptr), _ctx(nullptr), _close_flag(false),
              _width(0), _height(0) {}
        ~GpuWindowWindows() override { destroy(); }

        bool initialize(const char *title, int width, int height,
            bool fullscreen) noexcept override;
        void destroy() noexcept override;
        void poll_events() noexcept override;
        void swap_buffers() noexcept override;
        bool should_close() const noexcept override { return _close_flag; }
        int  get_width()  const noexcept override   { return _width; }
        int  get_height() const noexcept override   { return _height; }
        void request_close() noexcept { _close_flag = true; }
        void set_size(int width, int height) noexcept
        {
            _width = width;
            _height = height;
        }
};

static LRESULT CALLBACK gpur_window_proc(HWND hwnd, UINT msg, WPARAM wparam,
    LPARAM lparam)
{
    GpuWindowWindows *window = reinterpret_cast<GpuWindowWindows *>(
        GetWindowLongPtrA(hwnd, GWLP_USERDATA));

    switch (msg)
    {
        case WM_NCCREATE:
        {
            CREATESTRUCTA *create = reinterpret_cast<CREATESTRUCTA *>(lparam);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(create->lpCreateParams));
            return (TRUE);
        }
        case WM_CLOSE:
            if (window != nullptr)
                window->request_close();
            DestroyWindow(hwnd);
            return (0);
        case WM_ACTIVATE:
            if (LOWORD(wparam) == WA_INACTIVE)
            {
                ft_dumb_controls_win32_unregister_capture_window();
            }
            else
            {
                ft_dumb_controls_win32_register_capture_window(hwnd);
            }
            return (0);
        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE && window != nullptr)
            {
                window->request_close();
                DestroyWindow(hwnd);
                return (0);
            }
            break;
        case WM_DESTROY:
            ft_dumb_controls_win32_unregister_capture_window();
            PostQuitMessage(0);
            return (0);
        case WM_SIZE:
            if (window != nullptr)
                window->set_size(LOWORD(lparam), HIWORD(lparam));
            return (0);
        default:
            return (DefWindowProcA(hwnd, msg, wparam, lparam));
    }
    return (DefWindowProcA(hwnd, msg, wparam, lparam));
}

bool GpuWindowWindows::initialize(const char *title, int width, int height,
    bool fullscreen) noexcept
{
    static const char *class_name = "ft_vox_gpu_window";

    HINSTANCE instance = GetModuleHandleA(nullptr);
    WNDCLASSA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = gpur_window_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = class_name;

    if (RegisterClassA(&wc) == 0)
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return (false);
    }

    DWORD style = fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, style, FALSE);

    _width = width;
    _height = height;
    _close_flag = false;

    _hwnd = CreateWindowExA(0, class_name, title, style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
        rect.bottom - rect.top, nullptr, nullptr, instance, this);
    if (_hwnd == nullptr)
        return (false);

    _dc = GetDC(_hwnd);
    if (_dc == nullptr)
    {
        destroy();
        return (false);
    }

    PIXELFORMATDESCRIPTOR pfd;
    std::memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixel_format = ChoosePixelFormat(_dc, &pfd);
    if (pixel_format == 0 || SetPixelFormat(_dc, pixel_format, &pfd) == FALSE)
    {
        destroy();
        return (false);
    }

    _ctx = wglCreateContext(_dc);
    if (_ctx == nullptr || wglMakeCurrent(_dc, _ctx) == FALSE)
    {
        destroy();
        return (false);
    }

    if (!gpur_load_gl_functions())
    {
        destroy();
        return (false);
    }

    ft_dumb_controls_win32_register_capture_window(_hwnd);
    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);
    return (true);
}

void GpuWindowWindows::destroy() noexcept
{
    if (_ctx != nullptr)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(_ctx);
        _ctx = nullptr;
    }
    if (_dc != nullptr && _hwnd != nullptr)
    {
        ReleaseDC(_hwnd, _dc);
        _dc = nullptr;
    }
    if (_hwnd != nullptr)
    {
        ft_dumb_controls_win32_unregister_capture_window();
        DestroyWindow(_hwnd);
        _hwnd = nullptr;
    }
}

void GpuWindowWindows::poll_events() noexcept
{
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        if (msg.message == WM_QUIT)
            _close_flag = true;
    }
}

void GpuWindowWindows::swap_buffers() noexcept
{
    if (_dc != nullptr)
        SwapBuffers(_dc);
}

GpuWindow *GpuWindow::create() noexcept
{
    return (new (std::nothrow) GpuWindowWindows());
}

#endif
