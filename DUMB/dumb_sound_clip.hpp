#pragma once

#include <vector>
#include <stdint.h>
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"

struct ft_sound_spec;

class ft_sound_clip
{
    private:
        std::vector<uint8_t>     _data;
        ft_sound_spec            *_spec;
        pt_recursive_mutex       *_mutex = ft_nullptr;
        mutable bool             _thread_safe_enabled = false;

        int                      prepare_thread_safety(void) noexcept;
        void                     teardown_thread_safety(void) noexcept;

    public:
        ft_sound_clip(void);
        ~ft_sound_clip(void);

        int                      load_wav(const char *file_path);
        const uint8_t           *get_data(void) const;
        size_t                   get_size(void) const;
        const ft_sound_spec     *get_spec(void) const;

        int                      enable_thread_safety() noexcept;
        void                     disable_thread_safety() noexcept;
        bool                     is_thread_safe_enabled() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex       *runtime_mutex(void);
#endif
};
