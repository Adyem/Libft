#include "crafting.hpp"

ft_crafting::ft_crafting() noexcept
    : _recipes(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_map<int, ft_vector<int>> &ft_crafting::get_recipes() noexcept
{
    return (this->_recipes);
}

const ft_map<int, ft_vector<int>> &ft_crafting::get_recipes() const noexcept
{
    return (this->_recipes);
}

int ft_crafting::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_crafting::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void ft_crafting::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int ft_crafting::register_recipe(int recipe_id, ft_vector<int> &&ingredients) noexcept
{
    this->_error_code = ER_SUCCESS;
    this->_recipes.insert(recipe_id, ft_move(ingredients));
    if (this->_recipes.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_recipes.get_error());
        return (this->_error_code);
    }
    return (ER_SUCCESS);
}

int ft_crafting::craft_item(ft_inventory &inventory, int recipe_id, const ft_item &result) noexcept
{
    this->_error_code = ER_SUCCESS;
    Pair<int, ft_vector<int>> *recipe_entry = this->_recipes.find(recipe_id);
    if (!recipe_entry)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (GAME_GENERAL_ERROR);
    }

    ft_vector<int> &ingredients = recipe_entry->value;
    size_t index = 0;
    while (index < ingredients.size())
    {
        if (!inventory.has_item(ingredients[index]))
        {
            this->set_error(GAME_GENERAL_ERROR);
            return (GAME_GENERAL_ERROR);
        }
        ++index;
    }

    index = 0;
    while (index < ingredients.size())
    {
        int ingredient_id = ingredients[index];
        ft_map<int, ft_item> &items = inventory.get_items();
        Pair<int, ft_item> *item_ptr = items.end() - items.size();
        Pair<int, ft_item> *item_end = items.end();
        bool ingredient_removed = false;
        while (item_ptr != item_end && ingredient_removed == false)
        {
            if (item_ptr->value.get_item_id() == ingredient_id)
            {
                item_ptr->value.sub_from_stack(1);
                if (item_ptr->value.get_current_stack() == 0)
                    items.remove(item_ptr->key);
                ingredient_removed = true;
            }
            ++item_ptr;
        }
        ++index;
    }

    inventory.add_item(result);
    if (inventory.get_error() != ER_SUCCESS)
    {
        this->set_error(inventory.get_error());
        return (this->_error_code);
    }
    return (ER_SUCCESS);
}

