#ifndef GAME_EXPERIENCE_TABLE_HPP
# define GAME_EXPERIENCE_TABLE_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_experience_table
{
    private:
        int             *_levels;
        int             _count;
        mutable int     _error;
        mutable pt_mutex _mutex;

        void set_error(int err) const noexcept;
        bool is_valid(int count, const int *array) const noexcept;
        static int lock_pair(const ft_experience_table &first,
                const ft_experience_table &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);
        int resize_locked(int new_count, ft_unique_lock<pt_mutex> &guard,
                          bool validate_existing = true) noexcept;

    public:
        ft_experience_table(int count = 0) noexcept;
        ~ft_experience_table();
        ft_experience_table(const ft_experience_table &other) noexcept;
        ft_experience_table &operator=(const ft_experience_table &other) noexcept;
        ft_experience_table(ft_experience_table &&other) noexcept;
        ft_experience_table &operator=(ft_experience_table &&other) noexcept;

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
        int  check_for_error() const noexcept;
        int  get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
