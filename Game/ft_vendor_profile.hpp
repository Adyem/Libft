#ifndef FT_VENDOR_PROFILE_HPP
# define FT_VENDOR_PROFILE_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_vendor_profile
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t       _vendor_id;
        double    _buy_markup;
        double    _sell_multiplier;
        double    _tax_rate;
        mutable pt_recursive_mutex *_mutex;
        uint8_t   _initialised_state;
        static thread_local int32_t _last_error;


        static int32_t set_error(int32_t error_code) noexcept;

        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        int32_t unlock_internal(ft_bool lock_acquired) const noexcept;

    public:
        ft_vendor_profile() noexcept;
        virtual ~ft_vendor_profile() noexcept;
        ft_vendor_profile(const ft_vendor_profile &other) noexcept;
        ft_vendor_profile &operator=(const ft_vendor_profile &other) = delete;
        ft_vendor_profile(ft_vendor_profile &&other) noexcept;
        ft_vendor_profile &operator=(ft_vendor_profile &&other) = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_vendor_profile &other) noexcept;
        int32_t initialize(ft_vendor_profile &&other) noexcept;
        int32_t move(ft_vendor_profile &other) noexcept;
        int32_t initialize(int32_t vendor_id, double buy_markup,
            double sell_multiplier, double tax_rate) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t lock(ft_bool *lock_acquired) const noexcept;
        void unlock(ft_bool lock_acquired) const noexcept;

        int32_t get_vendor_id() const noexcept;
        void set_vendor_id(int32_t vendor_id) noexcept;

        double get_buy_markup() const noexcept;
        void set_buy_markup(double buy_markup) noexcept;

        double get_sell_multiplier() const noexcept;
        void set_sell_multiplier(double sell_multiplier) noexcept;

        double get_tax_rate() const noexcept;
        void set_tax_rate(double tax_rate) noexcept;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
