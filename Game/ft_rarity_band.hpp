#ifndef FT_RARITY_BAND_HPP
# define FT_RARITY_BAND_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_rarity_band
{
    private:
        int             _rarity;
        double          _value_multiplier;
        mutable int     _error_code;
        mutable pt_mutex _mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        void set_error(int error_code) const noexcept;
        static void record_operation_error_unlocked(int error_code);
        static int lock_pair(const ft_rarity_band &first, const ft_rarity_band &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_rarity_band() noexcept;
        ft_rarity_band(int rarity, double value_multiplier) noexcept;
        virtual ~ft_rarity_band() = default;
        ft_rarity_band(const ft_rarity_band &other) noexcept;
        ft_rarity_band &operator=(const ft_rarity_band &other) noexcept;
        ft_rarity_band(ft_rarity_band &&other) noexcept;
        ft_rarity_band &operator=(ft_rarity_band &&other) noexcept;

        int get_rarity() const noexcept;
        void set_rarity(int rarity) noexcept;

        double get_value_multiplier() const noexcept;
        void set_value_multiplier(double value_multiplier) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
