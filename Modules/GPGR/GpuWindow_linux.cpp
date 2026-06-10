#if !defined(_WIN32) && !defined(__APPLE__)

#include "GpuWindow_linux.hpp"

typedef GLXContext (*glx_create_ctx_fn)(Display *, GLXFBConfig, GLXContext,
    Bool, const int *);

class GpuWindowLinux : public GpuWindow
{
    Display    *_display;
    Window      _window;
    GLXContext  _ctx;
    Atom        _wm_delete;
    bool        _close_flag;
    int         _width;
    int         _height;
    int         _mouse_x;
    int         _mouse_y;
    bool        _mouse_clicked;
    bool        _settings_key;
    bool        _cursor_visible;
    bool        _grab_active;
    Cursor      _invisible_cursor;

    public:
        GpuWindowLinux() noexcept
            : _display(nullptr), _window(0), _ctx(nullptr), _wm_delete(0),
              _close_flag(false), _width(0), _height(0),
              _mouse_x(0), _mouse_y(0), _mouse_clicked(false),
              _settings_key(false), _cursor_visible(true),
              _grab_active(false), _invisible_cursor(0) {}
        ~GpuWindowLinux() override { destroy(); }

        bool initialize(const char *title, int width, int height,
            bool fullscreen) noexcept override;
        void destroy() noexcept override;
        void poll_events() noexcept override;
        void swap_buffers() noexcept override;
        bool should_close() const noexcept override { return _close_flag; }
        int  get_width()  const noexcept override   { return _width;      }
        int  get_height() const noexcept override   { return _height;     }
        int  get_mouse_x() const noexcept override  { return _mouse_x;    }
        int  get_mouse_y() const noexcept override  { return _mouse_y;    }
        bool was_mouse_clicked() const noexcept override { return _mouse_clicked; }
        void set_cursor_visible(bool v) noexcept override;
        bool was_settings_key_pressed() const noexcept override { return _settings_key; }
};

bool GpuWindowLinux::initialize(const char *title, int width, int height,
    bool fullscreen) noexcept
{
    _display = XOpenDisplay(nullptr);
    if (_display == nullptr)
        return (false);

    int screen = DefaultScreen(_display);

    int fb_attribs[] = {
        GLX_X_RENDERABLE,  True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE,      8,
        GLX_GREEN_SIZE,    8,
        GLX_BLUE_SIZE,     8,
        GLX_DEPTH_SIZE,    24,
        GLX_DOUBLEBUFFER,  True,
        None
    };

    int fb_count = 0;
    GLXFBConfig *configs = glXChooseFBConfig(_display, screen, fb_attribs,
        &fb_count);
    if (configs == nullptr || fb_count == 0)
    {
        XCloseDisplay(_display);
        _display = nullptr;
        return (false);
    }
    GLXFBConfig fb = configs[0];
    XFree(configs);

    XVisualInfo *vi = glXGetVisualFromFBConfig(_display, fb);
    if (vi == nullptr)
    {
        XCloseDisplay(_display);
        _display = nullptr;
        return (false);
    }

    _width  = fullscreen ? DisplayWidth(_display, screen)
                         : width;
    _height = fullscreen ? DisplayHeight(_display, screen)
                         : height;

    XSetWindowAttributes swa;
    std::memset(&swa, 0, sizeof(swa));
    swa.colormap   = XCreateColormap(_display, RootWindow(_display, vi->screen),
        vi->visual, AllocNone);
    swa.event_mask = ExposureMask | StructureNotifyMask
        | ButtonPressMask | ButtonReleaseMask
        | PointerMotionMask | KeyPressMask | KeyReleaseMask
        | FocusChangeMask;
    swa.border_pixel = 0;

    _window = XCreateWindow(_display,
        RootWindow(_display, vi->screen),
        0, 0,
        static_cast<unsigned int>(_width),
        static_cast<unsigned int>(_height),
        0, vi->depth, InputOutput, vi->visual,
        CWBorderPixel | CWColormap | CWEventMask, &swa);
    XFree(vi);

    if (_window == 0)
    {
        XCloseDisplay(_display);
        _display = nullptr;
        return (false);
    }

    XStoreName(_display, _window, title);
    _wm_delete = XInternAtom(_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(_display, _window, &_wm_delete, 1);

    if (fullscreen)
    {
        Atom wm_state = XInternAtom(_display, "_NET_WM_STATE", False);
        Atom wm_fs    = XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", False);
        XChangeProperty(_display, _window, wm_state, XA_ATOM, 32,
            PropModeReplace,
            reinterpret_cast<unsigned char *>(&wm_fs), 1);
    }

    XMapWindow(_display, _window);
    XFlush(_display);

    char blank_data = 0;
    Pixmap blank_pm = XCreateBitmapFromData(_display, _window, &blank_data, 1, 1);
    XColor blank_col;
    std::memset(&blank_col, 0, sizeof(blank_col));
    _invisible_cursor = XCreatePixmapCursor(_display, blank_pm, blank_pm,
        &blank_col, &blank_col, 0, 0);
    XFreePixmap(_display, blank_pm);

    glx_create_ctx_fn create_ctx =
        reinterpret_cast<glx_create_ctx_fn>(glXGetProcAddressARB(
            reinterpret_cast<const GLubyte *>("glXCreateContextAttribsARB")));

    if (create_ctx != nullptr)
    {
        int ctx_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };
        _ctx = create_ctx(_display, fb, nullptr, True, ctx_attribs);
    }
    else
    {
        _ctx = glXCreateNewContext(_display, fb, GLX_RGBA_TYPE, nullptr, True);
    }

    if (_ctx == nullptr)
    {
        XDestroyWindow(_display, _window);
        XCloseDisplay(_display);
        _display = nullptr;
        _window  = 0;
        return (false);
    }

    glXMakeCurrent(_display, _window, _ctx);

    if (!gpur_load_gl_functions())
    {
        glXMakeCurrent(_display, None, nullptr);
        glXDestroyContext(_display, _ctx);
        XDestroyWindow(_display, _window);
        XCloseDisplay(_display);
        _display = nullptr;
        _window  = 0;
        _ctx     = nullptr;
        return (false);
    }

    ft_dumb_controls_linux_register_window(
        static_cast<void *>(_display),
        _window);

    return (true);
}

void GpuWindowLinux::destroy() noexcept
{
    if (_display == nullptr)
        return ;
    if (_window != 0)
        ft_dumb_controls_linux_unregister_window(
            _window);
    if (_grab_active)
    {
        XUngrabPointer(_display, CurrentTime);
        _grab_active = false;
    }
    if (_invisible_cursor != 0)
    {
        XFreeCursor(_display, _invisible_cursor);
        _invisible_cursor = 0;
    }
    glXMakeCurrent(_display, None, nullptr);
    if (_ctx != nullptr)
    {
        glXDestroyContext(_display, _ctx);
        _ctx = nullptr;
    }
    if (_window != 0)
    {
        XDestroyWindow(_display, _window);
        _window = 0;
    }
    XCloseDisplay(_display);
    _display = nullptr;
}

void GpuWindowLinux::poll_events() noexcept
{
    _mouse_clicked = false;
    _settings_key  = false;
    while (XPending(_display) > 0)
    {
        XEvent ev;
        XNextEvent(_display, &ev);
        if (ev.type == ClientMessage)
        {
            if (static_cast<Atom>(ev.xclient.data.l[0]) == _wm_delete)
                _close_flag = true;
        }
        else if (ev.type == ButtonPress && ev.xbutton.button == Button1)
        {
            _mouse_clicked = true;
        }
        else if (ev.type == MotionNotify)
        {
            _mouse_x = ev.xmotion.x;
            _mouse_y = ev.xmotion.y;
        }
        else if (ev.type == ButtonPress || ev.type == ButtonRelease
            || ev.type == MotionNotify)
        {
            _mouse_x = ev.xbutton.x;
            _mouse_y = ev.xbutton.y;
        }
        else if (ev.type == KeyPress)
        {
            KeySym sym = XLookupKeysym(&ev.xkey, 0);
            if (sym == XK_m || sym == XK_M)
                _settings_key = true;
        }
        else if (ev.type == FocusOut)
        {
            if (_grab_active)
            {
                XUngrabPointer(_display, CurrentTime);
                _grab_active = false;
            }
        }
        else if (ev.type == FocusIn)
        {
            if (!_cursor_visible && _window != 0
                && _invisible_cursor != 0 && !_grab_active)
            {
                int r = XGrabPointer(_display, _window, False,
                    PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                    GrabModeAsync, GrabModeAsync,
                    _window, _invisible_cursor, CurrentTime);
                _grab_active = (r == GrabSuccess);
            }
        }
    }
}

void GpuWindowLinux::swap_buffers() noexcept
{
    glXSwapBuffers(_display, _window);
}

void GpuWindowLinux::set_cursor_visible(bool v) noexcept
{
    if (_display == nullptr || _window == 0) return;
    if (!v)
    {
        XDefineCursor(_display, _window, _invisible_cursor);
        XWarpPointer(_display, None, _window,
            0, 0, 0, 0, _width / 2, _height / 2);
        XFlush(_display);
        if (!_grab_active)
        {
            int r = XGrabPointer(_display, _window, False,
                PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync,
                _window, _invisible_cursor, CurrentTime);
            _grab_active = (r == GrabSuccess);
        }
    }
    else
    {
        if (_grab_active)
        {
            XUngrabPointer(_display, CurrentTime);
            _grab_active = false;
        }
        XUndefineCursor(_display, _window);
    }
    _cursor_visible = v;
    XFlush(_display);
}

GpuWindow *GpuWindow::create() noexcept
{
    return (new (std::nothrow) GpuWindowLinux());
}

#endif
