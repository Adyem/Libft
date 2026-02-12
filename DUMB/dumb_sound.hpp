#pragma once

#include <stdint.h>
#include <stddef.h>
#include <cstdint>
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
        uint8_t                      _initialized_state;

        static const uint8_t         _state_uninitialized = 0;
        static const uint8_t         _state_destroyed = 1;
        static const uint8_t         _state_initialized = 2;

        void                         abort_lifecycle_error(const char *method_name,
                                        const char *reason) const noexcept;
        void                         abort_if_not_initialized(const char *method_name) const noexcept;
        int                          prepare_thread_safety(void) noexcept;
        void                         teardown_thread_safety(void) noexcept;

    public:
        ft_sound_device(void);
        ft_sound_device(const ft_sound_device &other) = delete;
        ft_sound_device(ft_sound_device &&other) = delete;
        ft_sound_device &operator=(const ft_sound_device &other) = delete;
        ft_sound_device &operator=(ft_sound_device &&other) = delete;
        virtual ~ft_sound_device(void);

        int                          initialize(void);
        int                          initialize(const ft_sound_device &other);
        int                          initialize(ft_sound_device &&other);
        int                          destroy(void);
        int                          move(ft_sound_device &other);
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
