#ifndef GAME_RESISTANCE_HPP
# define GAME_RESISTANCE_HPP

#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_resistance
{
    private:
        int              _percent_value;
        int              _flat_value;
        mutable int      _error_code;
        mutable pt_recursive_mutex *_mutex;

        void set_error(int error) const noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_resistance() noexcept;
        ~ft_resistance() noexcept;
        ft_resistance(const ft_resistance &other) noexcept = delete;
        ft_resistance &operator=(const ft_resistance &other) noexcept = delete;
        ft_resistance(ft_resistance &&other) noexcept = delete;
        ft_resistance &operator=(ft_resistance &&other) noexcept = delete;

        int set_values(int percent_value, int flat_value) noexcept;
        int set_percent(int percent_value) noexcept;
        int set_flat(int flat_value) noexcept;
        int get_percent() const noexcept;
        int get_flat() const noexcept;
        int reset() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

        int         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
