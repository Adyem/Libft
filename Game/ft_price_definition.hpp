#ifndef FT_PRICE_DEFINITION_HPP
# define FT_PRICE_DEFINITION_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_price_definition
{
    private:
        int             _item_id;
        int             _rarity;
        int             _base_value;
        int             _minimum_value;
        int             _maximum_value;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_price_definition &first, const ft_price_definition &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_price_definition() noexcept;
        ft_price_definition(int item_id, int rarity, int base_value, int minimum_value, int maximum_value) noexcept;
        virtual ~ft_price_definition() = default;
        ft_price_definition(const ft_price_definition &other) noexcept;
        ft_price_definition &operator=(const ft_price_definition &other) noexcept;
        ft_price_definition(ft_price_definition &&other) noexcept;
        ft_price_definition &operator=(ft_price_definition &&other) noexcept;

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

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
