#include "game_crafting.hpp"

ft_crafting_ingredient::ft_crafting_ingredient() noexcept
    : _item_id(0), _count(0), _rarity(-1), _mutex()
{
    return ;
}

ft_crafting_ingredient::ft_crafting_ingredient(int item_id, int count, int rarity) noexcept
    : _item_id(item_id), _count(count), _rarity(rarity), _mutex()
{
    return ;
}

ft_crafting_ingredient::~ft_crafting_ingredient() noexcept
{
    return ;
}

ft_crafting_ingredient::ft_crafting_ingredient(const ft_crafting_ingredient &other) noexcept
    : _item_id(other._item_id), _count(other._count), _rarity(other._rarity), _mutex()
{
    return ;
}

ft_crafting_ingredient &ft_crafting_ingredient::operator=(
    const ft_crafting_ingredient &other) noexcept
{
    if (this == &other)
        return (*this);
    this->_item_id = other._item_id;
    this->_count = other._count;
    this->_rarity = other._rarity;
    return (*this);
}

ft_crafting_ingredient::ft_crafting_ingredient(ft_crafting_ingredient &&other) noexcept
    : _item_id(other._item_id), _count(other._count), _rarity(other._rarity), _mutex()
{
    return ;
}

ft_crafting_ingredient &ft_crafting_ingredient::operator=(
    ft_crafting_ingredient &&other) noexcept
{
    if (this == &other)
        return (*this);
    this->_item_id = other._item_id;
    this->_count = other._count;
    this->_rarity = other._rarity;
    return (*this);
}

int ft_crafting_ingredient::get_item_id() const noexcept
{
    return (this->_item_id);
}

void ft_crafting_ingredient::set_item_id(int item_id) noexcept
{
    this->_item_id = item_id;
    return ;
}

int ft_crafting_ingredient::get_count() const noexcept
{
    return (this->_count);
}

void ft_crafting_ingredient::set_count(int count) noexcept
{
    if (count < 0)
        return ;
    this->_count = count;
    return ;
}

int ft_crafting_ingredient::get_rarity() const noexcept
{
    return (this->_rarity);
}

void ft_crafting_ingredient::set_rarity(int rarity) noexcept
{
    if (rarity < -1)
        return ;
    this->_rarity = rarity;
    return ;
}

int ft_crafting_ingredient::get_error() const noexcept
{
    return (FT_ERR_SUCCESS);
}

const char *ft_crafting_ingredient::get_error_str() const noexcept
{
    return (ft_strerror(FT_ERR_SUCCESS));
}

ft_crafting::ft_crafting() noexcept
    : _recipes(), _mutex()
{
    return ;
}

ft_map<int, ft_vector<ft_crafting_ingredient>> &ft_crafting::get_recipes() noexcept
{
    return (this->_recipes);
}

const ft_map<int, ft_vector<ft_crafting_ingredient>> &ft_crafting::get_recipes() const noexcept
{
    return (this->_recipes);
}

int ft_crafting::get_error() const noexcept
{
    return (FT_ERR_SUCCESS);
}

const char *ft_crafting::get_error_str() const noexcept
{
    return (ft_strerror(FT_ERR_SUCCESS));
}

int ft_crafting::register_recipe(int recipe_id,
    ft_vector<ft_crafting_ingredient> &&ingredients) noexcept
{
    ft_vector<ft_crafting_ingredient> copied_ingredients;
    size_t ingredient_index;
    size_t ingredient_count;

    ingredient_index = 0;
    ingredient_count = ingredients.size();
    while (ingredient_index < ingredient_count)
    {
        copied_ingredients.push_back(ingredients[ingredient_index]);
        if (ft_vector<ft_crafting_ingredient>::last_operation_error() != FT_ERR_SUCCESS)
            return (ft_vector<ft_crafting_ingredient>::last_operation_error());
        ingredient_index++;
    }
    this->_recipes.insert(recipe_id, copied_ingredients);
    return (FT_ERR_SUCCESS);
}

int ft_crafting::craft_item(ft_inventory &inventory, int recipe_id,
    const ft_sharedptr<ft_item> &result) noexcept
{
    Pair<int, ft_vector<ft_crafting_ingredient>> *recipe_entry;
    ft_vector<ft_crafting_ingredient> *ingredients;
    size_t ingredient_index;

    if (!result)
        return (FT_ERR_INVALID_ARGUMENT);
    recipe_entry = this->_recipes.find(recipe_id);
    if (recipe_entry == this->_recipes.end())
        return (FT_ERR_NOT_FOUND);
    ingredients = &recipe_entry->value;
    ingredient_index = 0;
    while (ingredient_index < ingredients->size())
    {
        ft_crafting_ingredient &ingredient = (*ingredients)[ingredient_index];
        int have_count;

        if (ingredient.get_rarity() == -1)
            have_count = inventory.count_item(ingredient.get_item_id());
        else
            have_count = inventory.count_rarity(ingredient.get_rarity());
        if (have_count < ingredient.get_count())
            return (FT_ERR_GAME_GENERAL_ERROR);
        ingredient_index++;
    }
    ingredient_index = 0;
    while (ingredient_index < ingredients->size())
    {
        ft_crafting_ingredient &ingredient = (*ingredients)[ingredient_index];
        ft_map<int, ft_sharedptr<ft_item> > &items = inventory.get_items();
        Pair<int, ft_sharedptr<ft_item> > *item_ptr;
        Pair<int, ft_sharedptr<ft_item> > *item_end;
        int remaining;

        remaining = ingredient.get_count();
        item_ptr = items.end() - items.size();
        item_end = items.end();
        while (item_ptr != item_end && remaining > 0)
        {
            if (item_ptr->value)
            {
                if (item_ptr->value->get_item_id() == ingredient.get_item_id())
                {
                    if (ingredient.get_rarity() == -1
                        || item_ptr->value->get_rarity() == ingredient.get_rarity())
                    {
                        int remove_count;

                        remove_count = item_ptr->value->get_stack_size();
                        if (remove_count > remaining)
                            remove_count = remaining;
                        item_ptr->value->sub_from_stack(remove_count);
                        remaining -= remove_count;
                        if (item_ptr->value->get_stack_size() == 0)
                            items.remove(item_ptr->key);
                    }
                }
            }
            item_ptr++;
        }
        ingredient_index++;
    }
    return (inventory.add_item(result));
}
