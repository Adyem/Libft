#pragma once

#include "dumb_render.hpp"

struct ft_render_platform_result
{
    int error_code;
    int system_error_code;
};

ft_render_platform_result ft_render_platform_create_window(
    void **platform_state,
    ft_render_framebuffer *framebuffer,
    const ft_render_window_desc &desc
);

ft_render_platform_result ft_render_platform_destroy_window(
    void **platform_state,
    ft_render_framebuffer *framebuffer
);

ft_render_platform_result ft_render_platform_poll_events(
    void *platform_state,
    bool *should_close
);

ft_render_platform_result ft_render_platform_present(
    void *platform_state,
    ft_render_framebuffer *framebuffer
);

ft_render_platform_result ft_render_platform_set_fullscreen(
    void *platform_state,
    bool enabled
);

ft_render_platform_result ft_render_platform_get_primary_screen_size(
    ft_render_screen_size *size
);
