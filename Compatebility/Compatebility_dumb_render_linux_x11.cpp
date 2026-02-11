#if !defined(_WIN32) && !defined(__APPLE__)

#include "../DUMB/dumb_render_internal.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct ft_render_x11_state
{
    Display     *display;
    int32_t         screen;
    Window      window;
    GC          graphics_context;

    XImage      *image;
    int32_t         width;
    int32_t         height;

    bool        is_fullscreen;

    Atom        wm_delete_window;
    Atom        net_wm_state;
    Atom        net_wm_state_fullscreen;
};

static ft_render_platform_result ft_render_x11_create_image(
    ft_render_x11_state *state,
    ft_render_framebuffer *out_framebuffer
)
{
    int32_t         bytes_per_pixel;
    int32_t         bytes_per_row;
    char        *data;

    bytes_per_pixel = 4;
    bytes_per_row = state->width * bytes_per_pixel;

    data = static_cast<char *>(malloc(static_cast<ft_size_t>(bytes_per_row * state->height)));
    if (data == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_out_of_memory, 0 });
    }

    state->image = XCreateImage(
        state->display,
        DefaultVisual(state->display, state->screen),
        24,
        ZPixmap,
        0,
        data,
        static_cast<uint32_t>(state->width),
        static_cast<uint32_t>(state->height),
        32,
        bytes_per_row
    );

    if (state->image == NULL)
    {
        free(data);
        return ((ft_render_platform_result){ ft_render_error_platform_failure, errno });
    }

    out_framebuffer->width = state->width;
    out_framebuffer->height = state->height;
    out_framebuffer->pixels = reinterpret_cast<uint32_t *>(state->image->data);

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_get_primary_screen_size(ft_render_screen_size *out_size)
{
    Display *display;
    int32_t     screen;

    if (out_size == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    out_size->width = 0;
    out_size->height = 0;

    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_platform_failure, errno });
    }

    screen = DefaultScreen(display);

    out_size->width = DisplayWidth(display, screen);
    out_size->height = DisplayHeight(display, screen);

    XCloseDisplay(display);

    if (out_size->width <= 0 || out_size->height <= 0)
    {
        return ((ft_render_platform_result){ ft_render_error_platform_failure, errno });
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_create_window(
    void **out_platform_state,
    ft_render_framebuffer *out_framebuffer,
    const ft_render_window_desc &desc
)
{
    ft_render_x11_state  *state;
    int64_t                 event_mask;

    if (out_platform_state == NULL || out_framebuffer == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = static_cast<ft_render_x11_state *>(calloc(1, sizeof(ft_render_x11_state)));
    if (state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_out_of_memory, 0 });
    }

    state->display = XOpenDisplay(NULL);
    if (state->display == NULL)
    {
        free(state);
        return ((ft_render_platform_result){ ft_render_error_platform_failure, errno });
    }

    state->screen = DefaultScreen(state->display);
    state->width = desc.width;
    state->height = desc.height;
    state->is_fullscreen = false;

    event_mask = ExposureMask | StructureNotifyMask;

    state->window = XCreateSimpleWindow(
        state->display,
        RootWindow(state->display, state->screen),
        0,
        0,
        static_cast<uint32_t>(state->width),
        static_cast<uint32_t>(state->height),
        1,
        BlackPixel(state->display, state->screen),
        WhitePixel(state->display, state->screen)
    );

    XSelectInput(state->display, state->window, event_mask);
    XStoreName(state->display, state->window, desc.title);

    state->wm_delete_window = XInternAtom(state->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(state->display, state->window, &state->wm_delete_window, 1);

    state->net_wm_state = XInternAtom(state->display, "_NET_WM_STATE", False);
    state->net_wm_state_fullscreen = XInternAtom(state->display, "_NET_WM_STATE_FULLSCREEN", False);

    XMapWindow(state->display, state->window);
    state->graphics_context = DefaultGC(state->display, state->screen);

    {
        ft_render_platform_result img_result;

        img_result = ft_render_x11_create_image(state, out_framebuffer);
        if (img_result.error_code != ft_render_ok)
        {
            XDestroyWindow(state->display, state->window);
            XCloseDisplay(state->display);
            free(state);
            return (img_result);
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
    ft_render_x11_state  *state;

    if (platform_state == NULL || *platform_state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_ok, 0 });
    }

    state = static_cast<ft_render_x11_state *>(*platform_state);

    if (state->image != NULL)
    {
        if (state->image->data != NULL)
        {
            free(state->image->data);
            state->image->data = NULL;
        }
        XDestroyImage(state->image);
        state->image = NULL;
    }

    XDestroyWindow(state->display, state->window);
    XCloseDisplay(state->display);
    free(state);

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
    ft_render_x11_state  *state;
    XEvent               event;

    if (platform_state == NULL || out_should_close == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = static_cast<ft_render_x11_state *>(platform_state);
    *out_should_close = false;

    while (XPending(state->display) > 0)
    {
        XNextEvent(state->display, &event);

        if (event.type == ClientMessage)
        {
            if (static_cast<Atom>(event.xclient.data.l[0]) == state->wm_delete_window)
            {
                *out_should_close = true;
            }
        }
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

ft_render_platform_result ft_render_platform_present(
    void *platform_state,
    ft_render_framebuffer *framebuffer
)
{
    ft_render_x11_state  *state;

    if (platform_state == NULL || framebuffer == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = static_cast<ft_render_x11_state *>(platform_state);

    XPutImage(
        state->display,
        state->window,
        state->graphics_context,
        state->image,
        0,
        0,
        0,
        0,
        static_cast<uint32_t>(framebuffer->width),
        static_cast<uint32_t>(framebuffer->height)
    );

    XFlush(state->display);
    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

static void ft_render_x11_send_fullscreen_message(ft_render_x11_state *state, bool enabled)
{
    XEvent event;

    memset(&event, 0, sizeof(event));

    event.xclient.type = ClientMessage;
    event.xclient.send_event = True;
    event.xclient.window = state->window;
    event.xclient.message_type = state->net_wm_state;
    event.xclient.format = 32;
    if (enabled)
        event.xclient.data.l[0] = 1;
    else
        event.xclient.data.l[0] = 0;
    event.xclient.data.l[1] = static_cast<int64_t>(state->net_wm_state_fullscreen);
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 1;

    XSendEvent(
        state->display,
        DefaultRootWindow(state->display),
        False,
        SubstructureRedirectMask | SubstructureNotifyMask,
        &event
    );

    XFlush(state->display);
    return ;
}

ft_render_platform_result ft_render_platform_set_fullscreen(
    void *platform_state,
    bool enabled
)
{
    ft_render_x11_state  *state;

    if (platform_state == NULL)
    {
        return ((ft_render_platform_result){ ft_render_error_invalid_argument, 0 });
    }

    state = static_cast<ft_render_x11_state *>(platform_state);

    if (enabled == true && state->is_fullscreen == false)
    {
        ft_render_x11_send_fullscreen_message(state, true);
        state->is_fullscreen = true;
    }
    else if (enabled == false && state->is_fullscreen == true)
    {
        ft_render_x11_send_fullscreen_message(state, false);
        state->is_fullscreen = false;
    }

    return ((ft_render_platform_result){ ft_render_ok, 0 });
}

#endif
