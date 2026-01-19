#ifndef FT_CURRENCY_RATE_HPP
# define FT_CURRENCY_RATE_HPP

#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_currency_rate
{
    private:
        int             _currency_id;
        double          _rate_to_base;
        int             _display_precision;
        mutable int     _error_code;
        mutable pt_mutex _mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        void set_error(int error_code) const noexcept;
        static void record_operation_error_unlocked(int error_code);
        static int lock_pair(const ft_currency_rate &first, const ft_currency_rate &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_currency_rate() noexcept;
        ft_currency_rate(int currency_id, double rate_to_base, int display_precision) noexcept;
        virtual ~ft_currency_rate() = default;
        ft_currency_rate(const ft_currency_rate &other) noexcept;
        ft_currency_rate &operator=(const ft_currency_rate &other) noexcept;
        ft_currency_rate(ft_currency_rate &&other) noexcept;
        ft_currency_rate &operator=(ft_currency_rate &&other) noexcept;

        int get_currency_id() const noexcept;
        void set_currency_id(int currency_id) noexcept;

        double get_rate_to_base() const noexcept;
        void set_rate_to_base(double rate_to_base) noexcept;

        int get_display_precision() const noexcept;
        void set_display_precision(int display_precision) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
