#ifndef GAME_CRAFTING_HPP
# define GAME_CRAFTING_HPP

#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "game_inventory.hpp"
#include "game_item.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"

struct ft_crafting_ingredient
{
    int item_id;
    int count;
    int rarity;
};

class ft_crafting
{
    private:
        ft_map<int, ft_vector<ft_crafting_ingredient>> _recipes;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

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
