#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#if !defined(_WIN32) && !defined(__APPLE__)

#include "../DUMB/dumb_render_internal.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"

ft_render_platform_result ft_render_platform_get_primary_screen_size(
    ft_render_screen_size *out_size)
{
    if (out_size == ft_nullptr)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    out_size->width = 0;
    out_size->height = 0;
    return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
}

ft_render_platform_result ft_render_platform_create_window(
    void **out_platform_state,
    ft_render_framebuffer *out_framebuffer,
    const ft_render_window_desc &desc)
{
    (void)desc;
    if (out_platform_state == ft_nullptr || out_framebuffer == ft_nullptr)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    *out_platform_state = ft_nullptr;
    out_framebuffer->width = 0;
    out_framebuffer->height = 0;
    out_framebuffer->pixels = ft_nullptr;
    return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
}

ft_render_platform_result ft_render_platform_destroy_window(
    void **platform_state,
    ft_render_framebuffer *framebuffer)
{
    if (platform_state != ft_nullptr)
        *platform_state = ft_nullptr;
    if (framebuffer != ft_nullptr)
    {
        framebuffer->width = 0;
        framebuffer->height = 0;
        framebuffer->pixels = ft_nullptr;
    }
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_poll_events(
    void *platform_state,
    ft_bool *out_should_close)
{
    (void)platform_state;
    if (out_should_close == ft_nullptr)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    *out_should_close = FT_TRUE;
    return ((ft_render_platform_result){ FT_ERR_SUCCESS, 0 });
}

ft_render_platform_result ft_render_platform_present(
    void *platform_state,
    ft_render_framebuffer *framebuffer,
    const ft_render_depth_buffer *depth_buffer)
{
    (void)platform_state;
    (void)depth_buffer;
    if (framebuffer == ft_nullptr)
    {
        return ((ft_render_platform_result){ FT_ERR_INVALID_ARGUMENT, 0 });
    }
    return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
}

ft_render_platform_result ft_render_platform_set_fullscreen(
    void *platform_state,
    ft_bool enabled)
{
    (void)platform_state;
    (void)enabled;
    return ((ft_render_platform_result){ FT_ERR_INITIALIZATION_FAILED, 0 });
}

#endif
