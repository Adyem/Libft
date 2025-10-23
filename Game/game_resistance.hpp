#ifndef GAME_RESISTANCE_HPP
# define GAME_RESISTANCE_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_resistance
{
    private:
        int              _percent_value;
        int              _flat_value;
        mutable int      _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error) const noexcept;
        static int lock_pair(const ft_resistance &first,
                const ft_resistance &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_resistance() noexcept;
        ~ft_resistance() noexcept;
        ft_resistance(const ft_resistance &other) noexcept;
        ft_resistance &operator=(const ft_resistance &other) noexcept;
        ft_resistance(ft_resistance &&other) noexcept;
        ft_resistance &operator=(ft_resistance &&other) noexcept;

        int set_values(int percent_value, int flat_value) noexcept;
        int set_percent(int percent_value) noexcept;
        int set_flat(int flat_value) noexcept;
        int get_percent() const noexcept;
        int get_flat() const noexcept;
        int reset() noexcept;

        int         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
