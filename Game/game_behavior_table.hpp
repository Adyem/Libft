#ifndef GAME_BEHAVIOR_TABLE_HPP
# define GAME_BEHAVIOR_TABLE_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include "game_behavior_profile.hpp"
#include <stdint.h>

class ft_behavior_table
{
    private:
        ft_map<int, ft_behavior_profile> _profiles;
        pt_recursive_mutex                        *_mutex;
        uint8_t                          _initialized_state;
        static thread_local int           _last_error;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;
        void set_error(int error_code) const noexcept;

    public:
        ft_behavior_table() noexcept;
        ~ft_behavior_table() noexcept;
        ft_behavior_table(const ft_behavior_table &other) noexcept = delete;
        ft_behavior_table &operator=(const ft_behavior_table &other) noexcept = delete;
        ft_behavior_table(ft_behavior_table &&other) noexcept = delete;
        ft_behavior_table &operator=(ft_behavior_table &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_behavior_table &other) noexcept;
        int initialize(ft_behavior_table &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        ft_map<int, ft_behavior_profile> &get_profiles() noexcept;
        const ft_map<int, ft_behavior_profile> &get_profiles() const noexcept;
        void set_profiles(const ft_map<int, ft_behavior_profile> &profiles) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int register_profile(const ft_behavior_profile &profile) noexcept;
        int fetch_profile(int profile_id, ft_behavior_profile &profile) const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
