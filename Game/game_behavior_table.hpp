#ifndef GAME_BEHAVIOR_TABLE_HPP
# define GAME_BEHAVIOR_TABLE_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "ft_behavior_profile.hpp"

class ft_behavior_table
{
    private:
        ft_map<int, ft_behavior_profile> _profiles;
        mutable int                      _error_code;
        mutable pt_mutex                 _mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        void set_error(int error_code) const noexcept;
        static void record_operation_error_unlocked(int error_code);
        int clone_profiles_from(const ft_behavior_table &other) noexcept;
        static int lock_pair(const ft_behavior_table &first, const ft_behavior_table &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_behavior_table() noexcept;
        ~ft_behavior_table() noexcept;
        ft_behavior_table(const ft_behavior_table &other) noexcept;
        ft_behavior_table &operator=(const ft_behavior_table &other) noexcept;
        ft_behavior_table(ft_behavior_table &&other) noexcept;
        ft_behavior_table &operator=(ft_behavior_table &&other) noexcept;

        ft_map<int, ft_behavior_profile> &get_profiles() noexcept;
        const ft_map<int, ft_behavior_profile> &get_profiles() const noexcept;
        void set_profiles(const ft_map<int, ft_behavior_profile> &profiles) noexcept;

        int register_profile(const ft_behavior_profile &profile) noexcept;
        int fetch_profile(int profile_id, ft_behavior_profile &profile) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
