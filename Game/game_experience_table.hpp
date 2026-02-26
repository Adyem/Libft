#ifndef GAME_EXPERIENCE_TABLE_HPP
# define GAME_EXPERIENCE_TABLE_HPP

#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_experience_table
{
    private:
        int             *_levels;
        int             _count;
        static thread_local int _last_error;
        mutable pt_recursive_mutex *_mutex;

        void set_error(int err) const noexcept;
        bool is_valid(int count, const int *array) const noexcept;
        int resize_locked(int new_count,
                          bool validate_existing = true) noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_experience_table() noexcept;
        ~ft_experience_table();
        ft_experience_table(const ft_experience_table &other) noexcept = delete;
        ft_experience_table &operator=(const ft_experience_table &other) noexcept = delete;
        ft_experience_table(ft_experience_table &&other) noexcept = delete;
        ft_experience_table &operator=(ft_experience_table &&other) noexcept = delete;

        int  get_count() const noexcept;
        int  get_level(int experience) const noexcept;
        int  get_value(int index) const noexcept;
        void set_value(int index, int value) noexcept;
        int  set_levels(const int *levels, int count) noexcept;
        int  generate_levels_total(int count, int base,
                                   double multiplier) noexcept;
        int  generate_levels_scaled(int count, int base,
                                    double multiplier) noexcept;
        int  resize(int new_count) noexcept;
        int  enable_thread_safety() noexcept;
        int  disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int  check_for_error() const noexcept;
        int  get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
