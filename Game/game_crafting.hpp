#ifndef GAME_CRAFTING_HPP
# define GAME_CRAFTING_HPP

#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "game_inventory.hpp"
#include "game_item.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

struct ft_crafting_ingredient
{
    private:
        int             _item_id;
        int             _count;
        int             _rarity;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_crafting_ingredient &first,
                const ft_crafting_ingredient &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_crafting_ingredient() noexcept;
        ft_crafting_ingredient(int item_id, int count, int rarity) noexcept;
        virtual ~ft_crafting_ingredient() = default;
        ft_crafting_ingredient(const ft_crafting_ingredient &other) noexcept;
        ft_crafting_ingredient &operator=(const ft_crafting_ingredient &other) noexcept;
        ft_crafting_ingredient(ft_crafting_ingredient &&other) noexcept;
        ft_crafting_ingredient &operator=(ft_crafting_ingredient &&other) noexcept;

        int get_item_id() const noexcept;
        void set_item_id(int item_id) noexcept;

        int get_count() const noexcept;
        void set_count(int count) noexcept;

        int get_rarity() const noexcept;
        void set_rarity(int rarity) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_crafting
{
    private:
        ft_map<int, ft_vector<ft_crafting_ingredient>> _recipes;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_crafting &first, const ft_crafting &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_crafting() noexcept;
        virtual ~ft_crafting() = default;
        ft_crafting(const ft_crafting &other) noexcept;
        ft_crafting &operator=(const ft_crafting &other) noexcept;
        ft_crafting(ft_crafting &&other) noexcept;
        ft_crafting &operator=(ft_crafting &&other) noexcept;

        ft_map<int, ft_vector<ft_crafting_ingredient>>       &get_recipes() noexcept;
        const ft_map<int, ft_vector<ft_crafting_ingredient>> &get_recipes() const noexcept;

        int  get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int register_recipe(int recipe_id, ft_vector<ft_crafting_ingredient> &&ingredients) noexcept;
        int craft_item(ft_inventory &inventory, int recipe_id, const ft_sharedptr<ft_item> &result) noexcept;
};

#endif
