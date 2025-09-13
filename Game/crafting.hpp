#ifndef CRAFTING_HPP
#define CRAFTING_HPP

#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "inventory.hpp"
#include "item.hpp"
#include "../Errno/errno.hpp"

class ft_crafting
{
    private:
        ft_map<int, ft_vector<int>> _recipes;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    public:
        ft_crafting() noexcept;
        virtual ~ft_crafting() = default;

        ft_map<int, ft_vector<int>>       &get_recipes() noexcept;
        const ft_map<int, ft_vector<int>> &get_recipes() const noexcept;

        int  get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int register_recipe(int recipe_id, ft_vector<int> &&ingredients) noexcept;
        int craft_item(ft_inventory &inventory, int recipe_id, const ft_item &result) noexcept;
};

#endif
