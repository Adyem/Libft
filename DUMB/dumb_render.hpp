#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"

#ifndef FT_TYPES_HPP
# define FT_TYPES_HPP
# include <cstdint>

typedef uint64_t ft_size_t;

#endif

enum ft_render_error
{
    ft_render_ok = 0,
    ft_render_error_invalid_argument = 1,
    ft_render_error_not_initialized = 2,
    ft_render_error_platform_failure = 3,
    ft_render_error_out_of_memory = 4
};

enum ft_render_window_flags
{
    ft_render_window_flag_none = 0,
    ft_render_window_flag_resizable = 1 << 0,
    ft_render_window_flag_fullscreen = 1 << 1
};

struct ft_render_window_desc
{
    int             width;
    int             height;
    const char      *title;
    uint32_t        flags;
};

struct ft_render_framebuffer
{
    int             width;
    int             height;
    uint32_t        *pixels;
};

struct ft_render_screen_size
{
    int             width;
    int             height;
};

ft_render_screen_size  ft_render_get_primary_screen_size(void);

class ft_render_window
{
    private:

        pt_recursive_mutex           *_mutex = ft_nullptr;

        ft_render_framebuffer        _framebuffer;
        bool                         _is_initialized;
        bool                         _should_close;

        void                         *_platform_state;
        int                          prepare_thread_safety(void) noexcept;
        void                         teardown_thread_safety(void) noexcept;

    public:
        ft_render_window(void);
        ~ft_render_window(void);

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex           *runtime_mutex(void);
#endif

        int                          initialize(const ft_render_window_desc &desc);
        void                         shutdown(void);

        int                          poll_events(void);
        int                          present(void);

        ft_render_framebuffer        &framebuffer(void);
        int                          clear(uint32_t color);
        int                          put_pixel(int x, int y, uint32_t color);

        int                          set_fullscreen(bool enabled);

        int                          enable_thread_safety() noexcept;
        void                         disable_thread_safety() noexcept;
        bool                         is_thread_safe_enabled() const noexcept;

        bool                         should_close(void) const;
};
