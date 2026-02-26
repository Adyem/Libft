#ifndef FT_PRICE_DEFINITION_HPP
# define FT_PRICE_DEFINITION_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_price_definition
{
    private:
        int      _item_id;
        int      _rarity;
        int      _base_value;
        int      _minimum_value;
        int      _maximum_value;
        pt_recursive_mutex *_mutex;
        uint8_t  _initialized_state;
        static thread_local int _last_error;

        void set_error(int error_code) const noexcept;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_price_definition() noexcept;
        virtual ~ft_price_definition() noexcept;
        ft_price_definition(const ft_price_definition &other) noexcept = delete;
        ft_price_definition &operator=(const ft_price_definition &other) noexcept = delete;
        ft_price_definition(ft_price_definition &&other) noexcept = delete;
        ft_price_definition &operator=(ft_price_definition &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_price_definition &other) noexcept;
        int initialize(ft_price_definition &&other) noexcept;
        int initialize(int item_id, int rarity, int base_value,
            int minimum_value, int maximum_value) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_item_id() const noexcept;
        void set_item_id(int item_id) noexcept;

        int get_rarity() const noexcept;
        void set_rarity(int rarity) noexcept;

        int get_base_value() const noexcept;
        void set_base_value(int base_value) noexcept;

        int get_minimum_value() const noexcept;
        void set_minimum_value(int minimum_value) noexcept;

        int get_maximum_value() const noexcept;
        void set_maximum_value(int maximum_value) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
