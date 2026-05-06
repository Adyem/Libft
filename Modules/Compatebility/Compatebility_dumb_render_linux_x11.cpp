#if !defined(_WIN32) && !defined(__APPLE__)

#include "../DUMB/dumb_render_internal.hpp"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdbe.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct ft_render_x11_state
{
    Display             *display;
    int32_t             screen;
    Window              window;
    GC                  graphics_context;

    XImage              *image;
    int32_t             width;
    int32_t             height;

    ft_bool             is_fullscreen;
    ft_bool             has_back_buffer;
    XdbeBackBuffer      back_buffer;

    ft_bool             uses_glx;
    GLXContext          glx_context;
    Colormap            glx_colormap;
    GLuint              glx_texture;
    uint32_t            *glx_pixels;

    Atom                wm_delete_window;
    Atom                net_wm_state;
    Atom                net_wm_state_fullscreen;
};

static ft_bool ft_render_x11_string_contains(const char *string,
    const char *needle)
{
    if (string == NULL || needle == NULL)
    {
        return (FT_FALSE);
    }
    if (strstr(string, needle) != NULL)
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

static ft_bool ft_render_x11_should_try_glx(void)
{
    const char *enable_glx;

    enable_glx = getenv("FT_DUMB_ENABLE_GLX_PRESENT");
    if (enable_glx == NULL)
    {
        return (FT_FALSE);
    }
    if (enable_glx[0] == '1' && enable_glx[1] == '\0')
    {
        return (FT_TRUE);
    }
    if (strcmp(enable_glx, "true") == 0 || strcmp(enable_glx, "TRUE") == 0)
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

static ft_bool ft_render_x11_should_reject_gl_renderer(const char *vendor,
    const char *renderer)
{
    const char *force_software_gl;

    force_software_gl = getenv("LIBGL_ALWAYS_SOFTWARE");
    if (force_software_gl != NULL && force_software_gl[0] != '\0'
        && force_software_gl[0] != '0')
    {
        return (FT_TRUE);
    }
    if (ft_render_x11_string_contains(renderer, "llvmpipe") == FT_TRUE
        || ft_render_x11_string_contains(renderer, "softpipe") == FT_TRUE
        || ft_render_x11_string_contains(renderer, "Software Rasterizer")
            == FT_TRUE
        || ft_render_x11_string_contains(renderer, "SWR") == FT_TRUE
        || ft_render_x11_string_contains(vendor, "Mesa/X.org") == FT_TRUE)
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

static void ft_render_x11_setup_window_metadata(ft_render_x11_state *state,
    const char *title)
{
    state->wm_delete_window = XInternAtom(state->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(state->display, state->window, &state->wm_delete_window, 1);
    state->net_wm_state = XInternAtom(state->display, "_NET_WM_STATE", False);
    state->net_wm_state_fullscreen = XInternAtom(state->display,
        "_NET_WM_STATE_FULLSCREEN", False);
    XStoreName(state->display, state->window, title);
    return ;
}

static ft_render_platform_result ft_render_x11_create_image(
    ft_render_x11_state *state,
    ft_render_framebuffer *out_framebuffer
)
{
    int32_t             bytes_per_pixel;
    int32_t             bytes_per_row;
    char                *data;

    bytes_per_pixel = 4;
    bytes_per_row = state->width * bytes_per_pixel;
    data = static_cast<char *>(malloc(static_cast<ft_size_t>(bytes_per_row
        * state->height)));
    if (data == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_NO_MEMORY, 0 });
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
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    out_framebuffer->width = state->width;
    out_framebuffer->height = state->height;
    out_framebuffer->pixels = reinterpret_cast<uint32_t *>(state->image->data);
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

static ft_render_platform_result ft_render_x11_create_gl_framebuffer(
    ft_render_x11_state *state,
    ft_render_framebuffer *out_framebuffer
)
{
    ft_size_t           pixel_count;
    ft_size_t           byte_count;

    pixel_count = static_cast<ft_size_t>(state->width)
        * static_cast<ft_size_t>(state->height);
    byte_count = pixel_count * sizeof(uint32_t);
    state->glx_pixels = static_cast<uint32_t *>(malloc(byte_count));
    if (state->glx_pixels == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_NO_MEMORY, 0 });
    }
    memset(state->glx_pixels, 0, byte_count);
    out_framebuffer->width = state->width;
    out_framebuffer->height = state->height;
    out_framebuffer->pixels = state->glx_pixels;
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

static void ft_render_x11_enable_swap_interval(ft_render_x11_state *state)
{
    PFNGLXSWAPINTERVALEXTPROC  swap_interval_ext;
    PFNGLXSWAPINTERVALMESAPROC swap_interval_mesa;
    PFNGLXSWAPINTERVALSGIPROC  swap_interval_sgi;

    swap_interval_ext = reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(
        glXGetProcAddressARB(reinterpret_cast<const GLubyte *>(
            "glXSwapIntervalEXT"))
    );
    if (swap_interval_ext != NULL)
    {
        swap_interval_ext(state->display, state->window, 1);
        return ;
    }
    swap_interval_mesa = reinterpret_cast<PFNGLXSWAPINTERVALMESAPROC>(
        glXGetProcAddressARB(reinterpret_cast<const GLubyte *>(
            "glXSwapIntervalMESA"))
    );
    if (swap_interval_mesa != NULL)
    {
        (void)swap_interval_mesa(1U);
        return ;
    }
    swap_interval_sgi = reinterpret_cast<PFNGLXSWAPINTERVALSGIPROC>(
        glXGetProcAddressARB(reinterpret_cast<const GLubyte *>(
            "glXSwapIntervalSGI"))
    );
    if (swap_interval_sgi != NULL)
    {
        (void)swap_interval_sgi(1);
    }
    return ;
}

static void ft_render_x11_initialize_gl_state(ft_render_x11_state *state)
{
    glViewport(0, 0, state->width, state->height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenTextures(1, &state->glx_texture);
    glBindTexture(GL_TEXTURE_2D, state->glx_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state->width, state->height, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, state->glx_pixels);
    ft_render_x11_enable_swap_interval(state);
    return ;
}

static void ft_render_x11_destroy_gl_resources(ft_render_x11_state *state)
{
    if (state->display != NULL && state->glx_context != NULL)
    {
        if (state->glx_texture != 0U)
        {
            if (glXMakeCurrent(state->display, state->window,
                    state->glx_context) != False)
            {
                glDeleteTextures(1, &state->glx_texture);
                state->glx_texture = 0U;
            }
        }
        glXMakeCurrent(state->display, None, NULL);
        glXDestroyContext(state->display, state->glx_context);
        state->glx_context = NULL;
    }
    if (state->glx_pixels != NULL)
    {
        free(state->glx_pixels);
        state->glx_pixels = NULL;
    }
    if (state->display != NULL && state->glx_colormap != 0)
    {
        XFreeColormap(state->display, state->glx_colormap);
        state->glx_colormap = 0;
    }
    return ;
}

static ft_render_platform_result ft_render_x11_try_create_gl_window(
    ft_render_x11_state *state,
    ft_render_framebuffer *out_framebuffer,
    const ft_render_window_desc &desc
)
{
    XVisualInfo         *visual_info;
    XSetWindowAttributes window_attributes;
    long                event_mask;
    int32_t             glx_attributes[9];
    ft_render_platform_result framebuffer_result;
    const GLubyte       *vendor_string;
    const GLubyte       *renderer_string;

    glx_attributes[0] = GLX_RGBA;
    glx_attributes[1] = GLX_DOUBLEBUFFER;
    glx_attributes[2] = GLX_RED_SIZE;
    glx_attributes[3] = 8;
    glx_attributes[4] = GLX_GREEN_SIZE;
    glx_attributes[5] = 8;
    glx_attributes[6] = GLX_BLUE_SIZE;
    glx_attributes[7] = 8;
    glx_attributes[8] = None;
    visual_info = glXChooseVisual(state->display, state->screen, glx_attributes);
    if (visual_info == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
    }
    memset(&window_attributes, 0, sizeof(window_attributes));
    event_mask = ExposureMask | StructureNotifyMask;
    state->glx_colormap = XCreateColormap(state->display,
        RootWindow(state->display, state->screen), visual_info->visual,
        AllocNone);
    if (state->glx_colormap == 0)
    {
        XFree(visual_info);
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    window_attributes.colormap = state->glx_colormap;
    window_attributes.event_mask = event_mask;
    state->window = XCreateWindow(
        state->display,
        RootWindow(state->display, state->screen),
        0,
        0,
        static_cast<uint32_t>(state->width),
        static_cast<uint32_t>(state->height),
        0,
        visual_info->depth,
        InputOutput,
        visual_info->visual,
        CWColormap | CWEventMask,
        &window_attributes
    );
    if (state->window == 0)
    {
        XFreeColormap(state->display, state->glx_colormap);
        state->glx_colormap = 0;
        XFree(visual_info);
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    ft_render_x11_setup_window_metadata(state, desc.title);
    XMapWindow(state->display, state->window);
    state->glx_context = glXCreateContext(state->display, visual_info, NULL, True);
    XFree(visual_info);
    if (state->glx_context == NULL)
    {
        XDestroyWindow(state->display, state->window);
        state->window = 0;
        XFreeColormap(state->display, state->glx_colormap);
        state->glx_colormap = 0;
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    if (glXMakeCurrent(state->display, state->window, state->glx_context) == False)
    {
        glXDestroyContext(state->display, state->glx_context);
        state->glx_context = NULL;
        XDestroyWindow(state->display, state->window);
        state->window = 0;
        XFreeColormap(state->display, state->glx_colormap);
        state->glx_colormap = 0;
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    vendor_string = glGetString(GL_VENDOR);
    renderer_string = glGetString(GL_RENDERER);
    if (ft_render_x11_should_reject_gl_renderer(
            reinterpret_cast<const char *>(vendor_string),
            reinterpret_cast<const char *>(renderer_string)) == FT_TRUE)
    {
        glXMakeCurrent(state->display, None, NULL);
        glXDestroyContext(state->display, state->glx_context);
        state->glx_context = NULL;
        XDestroyWindow(state->display, state->window);
        state->window = 0;
        XFreeColormap(state->display, state->glx_colormap);
        state->glx_colormap = 0;
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
    }
    framebuffer_result = ft_render_x11_create_gl_framebuffer(state,
        out_framebuffer);
    if (framebuffer_result.error_code != FT_ERR_SUCCESS)
    {
        glXMakeCurrent(state->display, None, NULL);
        glXDestroyContext(state->display, state->glx_context);
        state->glx_context = NULL;
        XDestroyWindow(state->display, state->window);
        state->window = 0;
        XFreeColormap(state->display, state->glx_colormap);
        state->glx_colormap = 0;
        return (framebuffer_result);
    }
    state->uses_glx = FT_TRUE;
    ft_render_x11_initialize_gl_state(state);
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

static ft_render_platform_result ft_render_x11_create_software_window(
    ft_render_x11_state *state,
    ft_render_framebuffer *out_framebuffer,
    const ft_render_window_desc &desc
)
{
    long                    event_mask;
    ft_render_platform_result img_result;
    int32_t                 dbe_major_version;
    int32_t                 dbe_minor_version;

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
    if (state->window == 0)
    {
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    XSelectInput(state->display, state->window, event_mask);
    ft_render_x11_setup_window_metadata(state, desc.title);
    XMapWindow(state->display, state->window);
    state->graphics_context = XCreateGC(state->display, state->window, 0, NULL);
    if (state->graphics_context == NULL)
    {
        XDestroyWindow(state->display, state->window);
        state->window = 0;
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    state->has_back_buffer = FT_FALSE;
    state->back_buffer = 0;
    if (XdbeQueryExtension(state->display, &dbe_major_version,
            &dbe_minor_version) != 0)
    {
        state->back_buffer = XdbeAllocateBackBufferName(state->display,
            state->window, XdbeUndefined);
        if (state->back_buffer != 0)
        {
            state->has_back_buffer = FT_TRUE;
        }
    }
    img_result = ft_render_x11_create_image(state, out_framebuffer);
    if (img_result.error_code != FT_ERR_SUCCESS)
    {
        if (state->has_back_buffer == FT_TRUE)
        {
            XdbeDeallocateBackBufferName(state->display, state->back_buffer);
            state->back_buffer = 0;
            state->has_back_buffer = FT_FALSE;
        }
        XFreeGC(state->display, state->graphics_context);
        state->graphics_context = NULL;
        XDestroyWindow(state->display, state->window);
        state->window = 0;
        return (img_result);
    }
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_get_primary_screen_size(
    ft_render_screen_size *out_size)
{
    Display             *display;
    int32_t             screen;

    if (out_size == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    out_size->width = 0;
    out_size->height = 0;
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    screen = DefaultScreen(display);
    out_size->width = DisplayWidth(display, screen);
    out_size->height = DisplayHeight(display, screen);
    XCloseDisplay(display);
    if (out_size->width <= 0 || out_size->height <= 0)
    {
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_create_window(
    void **out_platform_state,
    ft_render_framebuffer *out_framebuffer,
    const ft_render_window_desc &desc
)
{
    ft_render_x11_state      *state;
    ft_render_platform_result create_result;

    if (out_platform_state == NULL || out_framebuffer == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    state = static_cast<ft_render_x11_state *>(calloc(1,
        sizeof(ft_render_x11_state)));
    if (state == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_NO_MEMORY, 0 });
    }
    state->display = XOpenDisplay(NULL);
    if (state->display == NULL)
    {
        free(state);
        return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, errno });
    }
    state->screen = DefaultScreen(state->display);
    state->width = desc.width;
    state->height = desc.height;
    state->is_fullscreen = FT_FALSE;
    if (ft_render_x11_should_try_glx() == FT_TRUE)
    {
        create_result = ft_render_x11_try_create_gl_window(state,
            out_framebuffer, desc);
        if (create_result.error_code == FT_ERR_SUCCESS)
        {
            *out_platform_state = state;
            return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
        }
    }
    create_result = ft_render_x11_create_software_window(state, out_framebuffer,
        desc);
    if (create_result.error_code != FT_ERR_SUCCESS)
    {
        XCloseDisplay(state->display);
        free(state);
        return (create_result);
    }
    *out_platform_state = state;
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_destroy_window(
    void **platform_state,
    ft_render_framebuffer *framebuffer
)
{
    ft_render_x11_state      *state;

    if (platform_state == NULL || *platform_state == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
    }
    state = static_cast<ft_render_x11_state *>(*platform_state);
    if (state->uses_glx == FT_TRUE)
    {
        ft_render_x11_destroy_gl_resources(state);
    }
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
    if (state->has_back_buffer == FT_TRUE)
    {
        XdbeDeallocateBackBufferName(state->display, state->back_buffer);
        state->back_buffer = 0;
        state->has_back_buffer = FT_FALSE;
    }
    if (state->graphics_context != NULL)
    {
        XFreeGC(state->display, state->graphics_context);
        state->graphics_context = NULL;
    }
    if (state->window != 0)
    {
        XDestroyWindow(state->display, state->window);
        state->window = 0;
    }
    XCloseDisplay(state->display);
    free(state);
    *platform_state = NULL;
    if (framebuffer != NULL)
    {
        framebuffer->width = 0;
        framebuffer->height = 0;
        framebuffer->pixels = NULL;
    }
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_poll_events(
    void *platform_state,
    ft_bool *out_should_close
)
{
    ft_render_x11_state      *state;
    XEvent                   event;

    if (platform_state == NULL || out_should_close == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    state = static_cast<ft_render_x11_state *>(platform_state);
    *out_should_close = FT_FALSE;
    while (XPending(state->display) > 0)
    {
        XNextEvent(state->display, &event);
        if (event.type == ClientMessage)
        {
            if (static_cast<Atom>(event.xclient.data.l[0])
                == state->wm_delete_window)
            {
                *out_should_close = FT_TRUE;
            }
        }
    }
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

static void ft_render_x11_present_glx(ft_render_x11_state *state,
    ft_render_framebuffer *framebuffer)
{
    glBindTexture(GL_TEXTURE_2D, state->glx_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, framebuffer->width,
        framebuffer->height, GL_BGRA, GL_UNSIGNED_BYTE, framebuffer->pixels);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
    glXSwapBuffers(state->display, state->window);
    return ;
}

static void ft_render_x11_present_software(ft_render_x11_state *state,
    ft_render_framebuffer *framebuffer)
{
    Drawable target_drawable;

    target_drawable = state->window;
    if (state->has_back_buffer == FT_TRUE)
    {
        target_drawable = state->back_buffer;
    }
    XPutImage(
        state->display,
        target_drawable,
        state->graphics_context,
        state->image,
        0,
        0,
        0,
        0,
        static_cast<uint32_t>(framebuffer->width),
        static_cast<uint32_t>(framebuffer->height)
    );
    if (state->has_back_buffer == FT_TRUE)
    {
        XdbeSwapInfo swap_info;

        swap_info.swap_window = state->window;
        swap_info.swap_action = XdbeUndefined;
        XdbeSwapBuffers(state->display, &swap_info, 1);
    }
    XFlush(state->display);
    return ;
}

ft_render_platform_result ft_render_platform_present(
    void *platform_state,
    ft_render_framebuffer *framebuffer,
    const ft_render_depth_buffer *depth_buffer
)
{
    ft_render_x11_state      *state;

    (void)depth_buffer;

    if (platform_state == NULL || framebuffer == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    state = static_cast<ft_render_x11_state *>(platform_state);
    if (state->uses_glx == FT_TRUE)
    {
        ft_render_x11_present_glx(state, framebuffer);
        return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
    }
    ft_render_x11_present_software(state, framebuffer);
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

static void ft_render_x11_send_fullscreen_message(ft_render_x11_state *state,
    ft_bool enabled)
{
    XEvent event;

    memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.send_event = True;
    event.xclient.window = state->window;
    event.xclient.message_type = state->net_wm_state;
    event.xclient.format = 32;
    if (enabled == FT_TRUE)
    {
        event.xclient.data.l[0] = 1;
    }
    else
    {
        event.xclient.data.l[0] = 0;
    }
    event.xclient.data.l[1] = static_cast<int64_t>(
        state->net_wm_state_fullscreen);
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
    ft_bool enabled
)
{
    ft_render_x11_state      *state;

    if (platform_state == NULL)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    state = static_cast<ft_render_x11_state *>(platform_state);
    if (enabled == FT_TRUE && state->is_fullscreen == FT_FALSE)
    {
        ft_render_x11_send_fullscreen_message(state, FT_TRUE);
        state->is_fullscreen = FT_TRUE;
    }
    else if (enabled == FT_FALSE && state->is_fullscreen == FT_TRUE)
    {
        ft_render_x11_send_fullscreen_message(state, FT_FALSE);
        state->is_fullscreen = FT_FALSE;
    }
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

#endif
