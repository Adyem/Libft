#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "dumb_sound_clip.hpp"

enum ft_sound_error
{
    ft_sound_ok = 0,
    ft_sound_error_invalid_argument = 1,
    ft_sound_error_not_initialized = 2,
    ft_sound_error_platform_failure = 3,
    ft_sound_error_out_of_memory = 4
};

struct ft_sound_spec
{
    int         freq;
    int         channels;
    int         samples;
    void        (*callback)(void *userdata, uint8_t *stream, int len);
    void        *userdata;
};

class ft_sound_device
{
    private:
        pt_recursive_mutex           *_mutex = ft_nullptr;
        mutable bool                 _thread_safe_enabled = false;

        int                          prepare_thread_safety(void) noexcept;
        void                         teardown_thread_safety(void) noexcept;

    public:
        ft_sound_device(void);
        virtual ~ft_sound_device(void);

        virtual int open(const ft_sound_spec *spec) = 0;
        virtual void close(void) = 0;

        virtual void pause(int pause_on) = 0;
        virtual void play(const ft_sound_clip *clip) = 0;
        virtual void stop(void) = 0;

        int                          enable_thread_safety() noexcept;
        void                         disable_thread_safety() noexcept;
        bool                         is_thread_safe_enabled() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex           *runtime_mutex(void);
#endif
};

ft_sound_device *ft_create_sound_device(void);
