#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include "item.hpp"
#include "../Template/map.hpp"
#include "../Errno/errno.hpp"

class ft_inventory
{
    private:
        ft_map<int, ft_item> _items;
        size_t              _capacity;
        int                 _next_slot;
        mutable int         _error;

        void set_error(int err) const noexcept;

    public:
        ft_inventory(size_t capacity = 0) noexcept;
        virtual ~ft_inventory() = default;

        ft_map<int, ft_item>       &get_items() noexcept;
        const ft_map<int, ft_item> &get_items() const noexcept;

        size_t get_capacity() const noexcept;
        void   resize(size_t capacity) noexcept;
        size_t get_used() const noexcept;
        bool   is_full() const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int  add_item(const ft_item &item) noexcept;
        void remove_item(int slot) noexcept;

        int  count_item(int item_id) const noexcept;
        bool has_item(int item_id) const noexcept;
};

#endif
