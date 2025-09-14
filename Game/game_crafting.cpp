#include "game_crafting.hpp"

ft_crafting::ft_crafting() noexcept
    : _recipes(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_crafting::ft_crafting(const ft_crafting &other) noexcept
    : _recipes(), _error_code(other._error_code)
{
    const Pair<int, ft_vector<ft_crafting_ingredient>> *entry = other._recipes.end() - other._recipes.size();
    const Pair<int, ft_vector<ft_crafting_ingredient>> *end = other._recipes.end();
    while (entry != end)
    {
        ft_vector<ft_crafting_ingredient> ingredients;
        size_t index = 0;
        while (index < entry->value.size())
        {
            ingredients.push_back(entry->value[index]);
            if (ingredients.get_error() != ER_SUCCESS)
            {
                this->set_error(ingredients.get_error());
                return ;
            }
            ++index;
        }
        this->_recipes.insert(entry->key, ft_move(ingredients));
        if (this->_recipes.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_recipes.get_error());
            return ;
        }
        ++entry;
    }
    return ;
}

ft_crafting &ft_crafting::operator=(const ft_crafting &other) noexcept
{
    if (this != &other)
    {
        this->_recipes.clear();
        const Pair<int, ft_vector<ft_crafting_ingredient>> *entry = other._recipes.end() - other._recipes.size();
        const Pair<int, ft_vector<ft_crafting_ingredient>> *end = other._recipes.end();
        while (entry != end)
        {
            ft_vector<ft_crafting_ingredient> ingredients;
            size_t index = 0;
            while (index < entry->value.size())
            {
                ingredients.push_back(entry->value[index]);
                if (ingredients.get_error() != ER_SUCCESS)
                {
                    this->set_error(ingredients.get_error());
                    break;
                }
                ++index;
            }
            this->_recipes.insert(entry->key, ft_move(ingredients));
            if (this->_recipes.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_recipes.get_error());
                break;
            }
            ++entry;
        }
        this->_error_code = other._error_code;
    }
    return (*this);
}

ft_crafting::ft_crafting(ft_crafting &&other) noexcept
    : _recipes(ft_move(other._recipes)), _error_code(other._error_code)
{
    if (this->_recipes.get_error() != ER_SUCCESS)
        this->set_error(this->_recipes.get_error());
    other._error_code = ER_SUCCESS;
    other._recipes.clear();
    return ;
}

ft_crafting &ft_crafting::operator=(ft_crafting &&other) noexcept
{
    if (this != &other)
    {
        this->_recipes = ft_move(other._recipes);
        this->_error_code = other._error_code;
        if (this->_recipes.get_error() != ER_SUCCESS)
            this->set_error(this->_recipes.get_error());
        other._error_code = ER_SUCCESS;
        other._recipes.clear();
    }
    return (*this);
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

int ft_crafting::register_recipe(int recipe_id, ft_vector<ft_crafting_ingredient> &&ingredients) noexcept
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

int ft_crafting::craft_item(ft_inventory &inventory, int recipe_id, const ft_sharedptr<ft_item> &result) noexcept
{
    this->_error_code = ER_SUCCESS;
    if (!result)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (GAME_GENERAL_ERROR);
    }
    if (result.get_error() != ER_SUCCESS)
    {
        this->set_error(result.get_error());
        return (this->_error_code);
    }
    if (result->get_error() != ER_SUCCESS)
    {
        this->set_error(result->get_error());
        return (this->_error_code);
    }
    Pair<int, ft_vector<ft_crafting_ingredient>> *recipe_entry = this->_recipes.find(recipe_id);
    if (!recipe_entry)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (GAME_GENERAL_ERROR);
    }

    ft_vector<ft_crafting_ingredient> &ingredients = recipe_entry->value;
    size_t index = 0;
    while (index < ingredients.size())
    {
        ft_crafting_ingredient &ingredient = ingredients[index];
        int have_count = 0;
        const ft_map<int, ft_sharedptr<ft_item> > &items = inventory.get_items();
        const Pair<int, ft_sharedptr<ft_item> > *item_ptr = items.end() - items.size();
        const Pair<int, ft_sharedptr<ft_item> > *item_end = items.end();
        while (item_ptr != item_end)
        {
            if (item_ptr->value)
            {
                if (item_ptr->value.get_error() != ER_SUCCESS)
                {
                    this->set_error(item_ptr->value.get_error());
                    return (this->_error_code);
                }
                if (item_ptr->value->get_error() != ER_SUCCESS)
                {
                    this->set_error(item_ptr->value->get_error());
                    return (this->_error_code);
                }
                if (item_ptr->value->get_item_id() == ingredient.item_id)
                {
                    if (ingredient.rarity == -1 || item_ptr->value->get_rarity() == ingredient.rarity)
                        have_count += item_ptr->value->get_stack_size();
                }
            }
            ++item_ptr;
        }
        if (have_count < ingredient.count)
        {
            this->set_error(GAME_GENERAL_ERROR);
            return (GAME_GENERAL_ERROR);
        }
        ++index;
    }

    index = 0;
    while (index < ingredients.size())
    {
        ft_crafting_ingredient &ingredient = ingredients[index];
        int remaining = ingredient.count;
        ft_map<int, ft_sharedptr<ft_item> > &items = inventory.get_items();
        Pair<int, ft_sharedptr<ft_item> > *item_ptr = items.end() - items.size();
        Pair<int, ft_sharedptr<ft_item> > *item_end = items.end();
        while (item_ptr != item_end && remaining > 0)
        {
            if (item_ptr->value)
            {
                if (item_ptr->value.get_error() != ER_SUCCESS)
                {
                    this->set_error(item_ptr->value.get_error());
                    return (this->_error_code);
                }
                if (item_ptr->value->get_error() != ER_SUCCESS)
                {
                    this->set_error(item_ptr->value->get_error());
                    return (this->_error_code);
                }
                if (item_ptr->value->get_item_id() == ingredient.item_id)
                {
                    if (ingredient.rarity == -1 || item_ptr->value->get_rarity() == ingredient.rarity)
                    {
                        int remove = item_ptr->value->get_stack_size();
                        if (remove > remaining)
                            remove = remaining;
                        item_ptr->value->sub_from_stack(remove);
                        remaining -= remove;
                        if (item_ptr->value->get_stack_size() == 0)
                            items.remove(item_ptr->key);
                    }
                }
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

