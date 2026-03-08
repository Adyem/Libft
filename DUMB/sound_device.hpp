#pragma once

#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "sound_clip.hpp"

struct ft_sound_spec
{
    int32_t     freq;
    int32_t     channels;
    int32_t     samples;
    void        (*callback)(void *userdata, uint8_t *stream, int32_t length);
    void        *userdata;
};

class ft_sound_device
{
 #ifdef LIBFT_TEST_BUILD
    public:
 #else
    private:
 #endif
        pt_recursive_mutex           *_mutex = ft_nullptr;
        uint8_t                      _initialised_state;

        int32_t                      prepare_thread_safety(void) noexcept;
        uint32_t                     teardown_thread_safety(void) noexcept;

    public:
        ft_sound_device(void);
        ft_sound_device(const ft_sound_device &other);
        ft_sound_device(ft_sound_device &&other);
        ft_sound_device &operator=(const ft_sound_device &other) = delete;
        ft_sound_device &operator=(ft_sound_device &&other) = delete;
        virtual ~ft_sound_device(void);

        uint32_t                     initialize(void);
        uint32_t                     initialize(const ft_sound_device &other);
        uint32_t                     initialize(ft_sound_device &&other);
        int32_t                      destroy(void);
        int32_t                      move(ft_sound_device &other);
        virtual int32_t open(const ft_sound_spec *spec) = 0;
        virtual void close(void) = 0;

        virtual void pause(int32_t pause_on) = 0;
        virtual void play(const ft_sound_clip *clip) = 0;
        virtual void stop(void) = 0;

        uint32_t                     enable_thread_safety() noexcept;
        uint32_t                     disable_thread_safety() noexcept;
        ft_bool                      is_thread_safe() const noexcept;

};

ft_sound_device *ft_create_sound_device(void);
