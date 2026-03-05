#ifndef GAME_RESISTANCE_HPP
# define GAME_RESISTANCE_HPP

#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_resistance
{
    private:
        int              _percent_value;
        int              _flat_value;
        mutable pt_recursive_mutex *_mutex;
        uint8_t          _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        static thread_local int _last_error;

        void set_error(int error) const noexcept;
        void abort_lifecycle_error(const char *method_name,
            const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_resistance() noexcept;
        ~ft_resistance() noexcept;
        int initialize() noexcept;
        int destroy() noexcept;
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
