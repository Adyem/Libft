#ifndef GAME_RESISTANCE_HPP
# define GAME_RESISTANCE_HPP

#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_resistance
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t              _percent_value;
        int32_t              _flat_value;
        mutable pt_recursive_mutex *_mutex;
        uint8_t          _initialised_state;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        int32_t unlock_internal(ft_bool lock_acquired) const noexcept;

    public:
        ft_resistance() noexcept;
        ft_resistance(const ft_resistance &other) noexcept;
        ft_resistance(ft_resistance &&other) noexcept;
        ~ft_resistance() noexcept;
        int32_t initialize() noexcept;
        int32_t initialize(const ft_resistance &other) noexcept;
        int32_t initialize(ft_resistance &&other) noexcept;
        int32_t move(ft_resistance &other) noexcept;
        int32_t destroy() noexcept;
        ft_resistance &operator=(const ft_resistance &other) noexcept = delete;
        ft_resistance &operator=(ft_resistance &&other) noexcept = delete;

        int32_t set_values(int32_t percent_value, int32_t flat_value) noexcept;
        int32_t set_percent(int32_t percent_value) noexcept;
        int32_t set_flat(int32_t flat_value) noexcept;
        int32_t get_percent() const noexcept;
        int32_t get_flat() const noexcept;
        int32_t reset() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        int32_t         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
