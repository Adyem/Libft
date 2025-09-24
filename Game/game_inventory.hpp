#ifndef GAME_INVENTORY_HPP
# define GAME_INVENTORY_HPP

#ifndef USE_INVENTORY_WEIGHT
# define USE_INVENTORY_WEIGHT 1
#endif

#ifndef USE_INVENTORY_SLOTS
# define USE_INVENTORY_SLOTS 1
#endif

#include "game_item.hpp"
#include "../Template/map.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"

class ft_inventory
{
    private:
        ft_map<int, ft_sharedptr<ft_item> > _items;
        size_t              _capacity;
        size_t              _used_slots;
        int                 _weight_limit;
        int                 _current_weight;
        int                 _next_slot;
        mutable int         _error;

        void set_error(int err) const noexcept;

    public:
        ft_inventory(size_t capacity = 0, int weight_limit = 0) noexcept;
        virtual ~ft_inventory() = default;
        ft_inventory(const ft_inventory &other) noexcept;
        ft_inventory &operator=(const ft_inventory &other) noexcept;
        ft_inventory(ft_inventory &&other) noexcept;
        ft_inventory &operator=(ft_inventory &&other) noexcept;

        ft_map<int, ft_sharedptr<ft_item> >       &get_items() noexcept;
        const ft_map<int, ft_sharedptr<ft_item> > &get_items() const noexcept;

        size_t get_capacity() const noexcept;
        void   resize(size_t capacity) noexcept;
        size_t get_used() const noexcept;
        void   set_used_slots(size_t used) noexcept;
        bool   is_full() const noexcept;
        int    get_weight_limit() const noexcept;
        void   set_weight_limit(int limit) noexcept;
        int    get_current_weight() const noexcept;
        void   set_current_weight(int weight) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int  add_item(const ft_sharedptr<ft_item> &item) noexcept;
        void remove_item(int slot) noexcept;

        int  count_item(int item_id) const noexcept;
        bool has_item(int item_id) const noexcept;

        int  count_rarity(int rarity) const noexcept;
        bool has_rarity(int rarity) const noexcept;
};

#endif
