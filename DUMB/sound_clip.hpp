#pragma once

#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include <vector>
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"

struct ft_sound_spec;

class ft_sound_clip
{
 #ifdef LIBFT_TEST_BUILD
    public:
 #else
    private:
 #endif
        std::vector<uint8_t>     _data;
        ft_sound_spec            *_spec;
        pt_recursive_mutex       *_mutex = ft_nullptr;
        uint8_t                  _initialised_state;

        int32_t                  prepare_thread_safety(void) noexcept;
        uint32_t                 teardown_thread_safety(void) noexcept;

    public:
        ft_sound_clip(void);
        ft_sound_clip(const ft_sound_clip &other);
        ft_sound_clip(ft_sound_clip &&other);
        ft_sound_clip &operator=(const ft_sound_clip &other) = delete;
        ft_sound_clip &operator=(ft_sound_clip &&other) = delete;
        ~ft_sound_clip(void);

        uint32_t                 initialize(void);
        uint32_t                 initialize(const ft_sound_clip &other);
        uint32_t                 initialize(ft_sound_clip &&other);
        int32_t                  destroy(void);
        int32_t                  move(ft_sound_clip &other);
        int32_t                  load_wav(const char *file_path);
        const uint8_t           *get_data(void) const;
        ft_size_t                get_size(void) const;
        const ft_sound_spec     *get_spec(void) const;

        uint32_t                 enable_thread_safety() noexcept;
        uint32_t                 disable_thread_safety() noexcept;
        ft_bool                  is_thread_safe() const noexcept;

};
