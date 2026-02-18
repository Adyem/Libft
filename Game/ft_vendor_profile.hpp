#ifndef FT_VENDOR_PROFILE_HPP
# define FT_VENDOR_PROFILE_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_vendor_profile
{
    private:
        int       _vendor_id;
        double    _buy_markup;
        double    _sell_multiplier;
        double    _tax_rate;
        pt_mutex *_mutex;
        uint8_t   _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_vendor_profile() noexcept;
        ft_vendor_profile(int vendor_id, double buy_markup,
            double sell_multiplier, double tax_rate) noexcept;
        virtual ~ft_vendor_profile() noexcept;
        ft_vendor_profile(const ft_vendor_profile &other) = delete;
        ft_vendor_profile &operator=(const ft_vendor_profile &other) = delete;
        ft_vendor_profile(ft_vendor_profile &&other) = delete;
        ft_vendor_profile &operator=(ft_vendor_profile &&other) = delete;

        int initialize() noexcept;
        int initialize(const ft_vendor_profile &other) noexcept;
        int initialize(ft_vendor_profile &&other) noexcept;
        int initialize(int vendor_id, double buy_markup,
            double sell_multiplier, double tax_rate) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_vendor_id() const noexcept;
        void set_vendor_id(int vendor_id) noexcept;

        double get_buy_markup() const noexcept;
        void set_buy_markup(double buy_markup) noexcept;

        double get_sell_multiplier() const noexcept;
        void set_sell_multiplier(double sell_multiplier) noexcept;

        double get_tax_rate() const noexcept;
        void set_tax_rate(double tax_rate) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
