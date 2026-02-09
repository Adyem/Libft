#ifndef FT_VENDOR_PROFILE_HPP
# define FT_VENDOR_PROFILE_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_vendor_profile
{
    private:
        int             _vendor_id;
        double          _buy_markup;
        double          _sell_multiplier;
        double          _tax_rate;
        mutable int     _error_code;
        mutable pt_mutex _mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        void set_error(int error_code) const noexcept;
        static void record_operation_error_unlocked(int error_code);
        static int lock_pair(const ft_vendor_profile &first, const ft_vendor_profile &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_vendor_profile() noexcept;
        ft_vendor_profile(int vendor_id, double buy_markup, double sell_multiplier, double tax_rate) noexcept;
        virtual ~ft_vendor_profile() = default;
        ft_vendor_profile(const ft_vendor_profile &other) noexcept;
        ft_vendor_profile &operator=(const ft_vendor_profile &other) noexcept;
        ft_vendor_profile(ft_vendor_profile &&other) noexcept;
        ft_vendor_profile &operator=(ft_vendor_profile &&other) noexcept;

        int get_vendor_id() const noexcept;
        void set_vendor_id(int vendor_id) noexcept;

        double get_buy_markup() const noexcept;
        void set_buy_markup(double buy_markup) noexcept;

        double get_sell_multiplier() const noexcept;
        void set_sell_multiplier(double sell_multiplier) noexcept;

        double get_tax_rate() const noexcept;
        void set_tax_rate(double tax_rate) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
