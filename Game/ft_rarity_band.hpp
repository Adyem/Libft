#ifndef FT_RARITY_BAND_HPP
# define FT_RARITY_BAND_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_rarity_band
{
    private:
        int      _rarity;
        double   _value_multiplier;
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
        ft_rarity_band() noexcept;
        virtual ~ft_rarity_band() noexcept;
        ft_rarity_band(const ft_rarity_band &other) noexcept = delete;
        ft_rarity_band &operator=(const ft_rarity_band &other) noexcept = delete;
        ft_rarity_band(ft_rarity_band &&other) noexcept = delete;
        ft_rarity_band &operator=(ft_rarity_band &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_rarity_band &other) noexcept;
        int initialize(ft_rarity_band &&other) noexcept;
        int initialize(int rarity, double value_multiplier) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_rarity() const noexcept;
        void set_rarity(int rarity) noexcept;

        double get_value_multiplier() const noexcept;
        void set_value_multiplier(double value_multiplier) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
