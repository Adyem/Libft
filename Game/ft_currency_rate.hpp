#ifndef FT_CURRENCY_RATE_HPP
# define FT_CURRENCY_RATE_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_currency_rate
{
    private:
        int      _currency_id;
        double   _rate_to_base;
        int      _display_precision;
        pt_recursive_mutex *_mutex;
        uint8_t  _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_currency_rate() noexcept;
        virtual ~ft_currency_rate() noexcept;
        ft_currency_rate(const ft_currency_rate &other) noexcept = delete;
        ft_currency_rate &operator=(const ft_currency_rate &other) noexcept = delete;
        ft_currency_rate(ft_currency_rate &&other) noexcept = delete;
        ft_currency_rate &operator=(ft_currency_rate &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_currency_rate &other) noexcept;
        int initialize(ft_currency_rate &&other) noexcept;
        int initialize(int currency_id, double rate_to_base,
            int display_precision) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_currency_id() const noexcept;
        void set_currency_id(int currency_id) noexcept;

        double get_rate_to_base() const noexcept;
        void set_rate_to_base(double rate_to_base) noexcept;

        int get_display_precision() const noexcept;
        void set_display_precision(int display_precision) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
