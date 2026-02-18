#include "game_data_catalog.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_data_catalog_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_data_catalog_unlock_guard(ft_unique_lock<pt_mutex> &guard)
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

static void game_data_catalog_copy_crafting_vector(const ft_vector<ft_crafting_ingredient> &source,
        ft_vector<ft_crafting_ingredient> &destination)
{
    ft_vector<ft_crafting_ingredient>::const_iterator entry;
    ft_vector<ft_crafting_ingredient>::const_iterator end;

    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

static void game_data_catalog_copy_loadout_vector(const ft_vector<ft_loadout_entry> &source,
        ft_vector<ft_loadout_entry> &destination)
{
    ft_vector<ft_loadout_entry>::const_iterator entry;
    ft_vector<ft_loadout_entry>::const_iterator end;

    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

int ft_item_definition::lock_pair(const ft_item_definition &first,
        const ft_item_definition &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_item_definition *ordered_first;
    const ft_item_definition *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_item_definition *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_data_catalog_sleep_backoff();
    }
}

ft_item_definition::ft_item_definition() noexcept
    : _item_id(0), _rarity(0), _max_stack(0), _width(0), _height(0), _weight(0), _slot_requirement(0)
{
    return ;
}

ft_item_definition::ft_item_definition(int item_id, int rarity, int max_stack, int width, int height, int weight, int slot_requirement) noexcept
    : _item_id(item_id), _rarity(rarity), _max_stack(max_stack), _width(width), _height(height), _weight(weight),
      _slot_requirement(slot_requirement)
{
    return ;
}

ft_item_definition::ft_item_definition(const ft_item_definition &other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_max_stack = other._max_stack;
    this->_width = other._width;
    this->_height = other._height;
    this->_weight = other._weight;
    this->_slot_requirement = other._slot_requirement;
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_item_definition &ft_item_definition::operator=(const ft_item_definition &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_item_definition::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_max_stack = other._max_stack;
    this->_width = other._width;
    this->_height = other._height;
    this->_weight = other._weight;
    this->_slot_requirement = other._slot_requirement;
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}

ft_item_definition::ft_item_definition(ft_item_definition &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_max_stack = other._max_stack;
    this->_width = other._width;
    this->_height = other._height;
    this->_weight = other._weight;
    this->_slot_requirement = other._slot_requirement;
    other._item_id = 0;
    other._rarity = 0;
    other._max_stack = 0;
    other._width = 0;
    other._height = 0;
    other._weight = 0;
    other._slot_requirement = 0;
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_item_definition &ft_item_definition::operator=(ft_item_definition &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_item_definition::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_max_stack = other._max_stack;
    this->_width = other._width;
    this->_height = other._height;
    this->_weight = other._weight;
    this->_slot_requirement = other._slot_requirement;
    other._item_id = 0;
    other._rarity = 0;
    other._max_stack = 0;
    other._width = 0;
    other._height = 0;
    other._weight = 0;
    other._slot_requirement = 0;
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}

int ft_item_definition::get_item_id() const noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    identifier = this->_item_id;
    game_data_catalog_unlock_guard(guard);
    return (identifier);
}

void ft_item_definition::set_item_id(int item_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_item_id = item_id;
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_item_definition::get_rarity() const noexcept
{
    int rarity_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    rarity_value = this->_rarity;
    game_data_catalog_unlock_guard(guard);
    return (rarity_value);
}

void ft_item_definition::set_rarity(int rarity) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_rarity = rarity;
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_item_definition::get_max_stack() const noexcept
{
    int value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    value = this->_max_stack;
    game_data_catalog_unlock_guard(guard);
    return (value);
}

void ft_item_definition::set_max_stack(int max_stack) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_max_stack = max_stack;
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_item_definition::get_width() const noexcept
{
    int width_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    width_value = this->_width;
    game_data_catalog_unlock_guard(guard);
    return (width_value);
}

void ft_item_definition::set_width(int width) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_width = width;
    game_data_catalog_unlock_guard(guard);
    return ;
}


int ft_item_definition::get_height() const noexcept
{
    int height_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    height_value = this->_height;
    game_data_catalog_unlock_guard(guard);
    return (height_value);
}

void ft_item_definition::set_height(int height) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_height = height;
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_item_definition::get_weight() const noexcept
{
    int weight_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    weight_value = this->_weight;
    game_data_catalog_unlock_guard(guard);
    return (weight_value);
}

void ft_item_definition::set_weight(int weight) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_weight = weight;
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_item_definition::get_slot_requirement() const noexcept
{
    int slot_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    slot_value = this->_slot_requirement;
    game_data_catalog_unlock_guard(guard);
    return (slot_value);
}

void ft_item_definition::set_slot_requirement(int slot_requirement) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_slot_requirement = slot_requirement;
    game_data_catalog_unlock_guard(guard);
    return ;
}



int ft_recipe_blueprint::lock_pair(const ft_recipe_blueprint &first,
        const ft_recipe_blueprint &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_recipe_blueprint *ordered_first;
    const ft_recipe_blueprint *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_recipe_blueprint *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_data_catalog_sleep_backoff();
    }
}

ft_recipe_blueprint::ft_recipe_blueprint() noexcept
    : _recipe_id(0), _result_item_id(0)
{
    return ;
}

ft_recipe_blueprint::ft_recipe_blueprint(int recipe_id, int result_item_id,
    const ft_vector<ft_crafting_ingredient> &ingredients) noexcept
    : _recipe_id(recipe_id), _result_item_id(result_item_id)
{
    game_data_catalog_copy_crafting_vector(ingredients, this->_ingredients);
    return ;
}

ft_recipe_blueprint::ft_recipe_blueprint(const ft_recipe_blueprint &other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_recipe_id = other._recipe_id;
    this->_result_item_id = other._result_item_id;
    game_data_catalog_copy_crafting_vector(other._ingredients, this->_ingredients);
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_recipe_blueprint &ft_recipe_blueprint::operator=(const ft_recipe_blueprint &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_recipe_blueprint::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_recipe_id = other._recipe_id;
    this->_result_item_id = other._result_item_id;
    game_data_catalog_copy_crafting_vector(other._ingredients, this->_ingredients);
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}

ft_recipe_blueprint::ft_recipe_blueprint(ft_recipe_blueprint &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_recipe_id = other._recipe_id;
    this->_result_item_id = other._result_item_id;
    game_data_catalog_copy_crafting_vector(other._ingredients, this->_ingredients);
    other._recipe_id = 0;
    other._result_item_id = 0;
    other._ingredients.clear();
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_recipe_blueprint &ft_recipe_blueprint::operator=(ft_recipe_blueprint &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_recipe_blueprint::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_recipe_id = other._recipe_id;
    this->_result_item_id = other._result_item_id;
    game_data_catalog_copy_crafting_vector(other._ingredients, this->_ingredients);
    other._recipe_id = 0;
    other._result_item_id = 0;
    other._ingredients.clear();
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}


int ft_recipe_blueprint::get_recipe_id() const noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    identifier = this->_recipe_id;
    game_data_catalog_unlock_guard(guard);
    return (identifier);
}

void ft_recipe_blueprint::set_recipe_id(int recipe_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_recipe_id = recipe_id;
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_recipe_blueprint::get_result_item_id() const noexcept
{
    int item_id;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        game_data_catalog_unlock_guard(guard);
        return (guard.get_error());
    }
    item_id = this->_result_item_id;
    game_data_catalog_unlock_guard(guard);
    return (item_id);
}

void ft_recipe_blueprint::set_result_item_id(int result_item_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_result_item_id = result_item_id;
    game_data_catalog_unlock_guard(guard);
    return ;
}

ft_vector<ft_crafting_ingredient> &ft_recipe_blueprint::get_ingredients() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (this->_ingredients);
    }
    game_data_catalog_unlock_guard(guard);
    return (this->_ingredients);
}

const ft_vector<ft_crafting_ingredient> &ft_recipe_blueprint::get_ingredients() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (this->_ingredients);
    }
    game_data_catalog_unlock_guard(guard);
    return (this->_ingredients);
}

void ft_recipe_blueprint::set_ingredients(const ft_vector<ft_crafting_ingredient> &ingredients) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    game_data_catalog_copy_crafting_vector(ingredients, this->_ingredients);
    game_data_catalog_unlock_guard(guard);
    return ;
}


int ft_loadout_entry::lock_pair(const ft_loadout_entry &first,
        const ft_loadout_entry &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_loadout_entry *ordered_first;
    const ft_loadout_entry *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_loadout_entry *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_data_catalog_sleep_backoff();
    }
}

ft_loadout_entry::ft_loadout_entry() noexcept
    : _slot(0), _item_id(0), _quantity(0)
{
    return ;
}

ft_loadout_entry::ft_loadout_entry(int slot, int item_id, int quantity) noexcept
    : _slot(slot), _item_id(item_id), _quantity(quantity)
{
    return ;
}

ft_loadout_entry::ft_loadout_entry(const ft_loadout_entry &other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_slot = other._slot;
    this->_item_id = other._item_id;
    this->_quantity = other._quantity;
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_loadout_entry &ft_loadout_entry::operator=(const ft_loadout_entry &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_loadout_entry::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_slot = other._slot;
    this->_item_id = other._item_id;
    this->_quantity = other._quantity;
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}

ft_loadout_entry::ft_loadout_entry(ft_loadout_entry &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_slot = other._slot;
    this->_item_id = other._item_id;
    this->_quantity = other._quantity;
    other._slot = 0;
    other._item_id = 0;
    other._quantity = 0;
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_loadout_entry &ft_loadout_entry::operator=(ft_loadout_entry &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_loadout_entry::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_slot = other._slot;
    this->_item_id = other._item_id;
    this->_quantity = other._quantity;
    other._slot = 0;
    other._item_id = 0;
    other._quantity = 0;
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}


int ft_loadout_entry::get_slot() const noexcept
{
    int slot_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    slot_value = this->_slot;
    game_data_catalog_unlock_guard(guard);
    return (slot_value);
}

void ft_loadout_entry::set_slot(int slot) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_slot = slot;
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_loadout_entry::get_item_id() const noexcept
{
    int item_id;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    item_id = this->_item_id;
    game_data_catalog_unlock_guard(guard);
    return (item_id);
}

void ft_loadout_entry::set_item_id(int item_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_item_id = item_id;
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_loadout_entry::get_quantity() const noexcept
{
    int quantity_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    quantity_value = this->_quantity;
    game_data_catalog_unlock_guard(guard);
    return (quantity_value);
}

void ft_loadout_entry::set_quantity(int quantity) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_quantity = quantity;
    game_data_catalog_unlock_guard(guard);
    return ;
}


int ft_loadout_blueprint::lock_pair(const ft_loadout_blueprint &first,
        const ft_loadout_blueprint &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_loadout_blueprint *ordered_first;
    const ft_loadout_blueprint *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_loadout_blueprint *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_data_catalog_sleep_backoff();
    }
}

ft_loadout_blueprint::ft_loadout_blueprint() noexcept
    : _loadout_id(0)
{
    return ;
}

ft_loadout_blueprint::ft_loadout_blueprint(int loadout_id, const ft_vector<ft_loadout_entry> &entries) noexcept
    : _loadout_id(loadout_id)
{
    game_data_catalog_copy_loadout_vector(entries, this->_entries);
    return ;
}

ft_loadout_blueprint::ft_loadout_blueprint(const ft_loadout_blueprint &other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_loadout_id = other._loadout_id;
    game_data_catalog_copy_loadout_vector(other._entries, this->_entries);
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_loadout_blueprint &ft_loadout_blueprint::operator=(const ft_loadout_blueprint &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_loadout_blueprint::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_loadout_id = other._loadout_id;
    game_data_catalog_copy_loadout_vector(other._entries, this->_entries);
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}

ft_loadout_blueprint::ft_loadout_blueprint(ft_loadout_blueprint &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_loadout_id = other._loadout_id;
    game_data_catalog_copy_loadout_vector(other._entries, this->_entries);
    other._loadout_id = 0;
    other._entries.clear();
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_loadout_blueprint &ft_loadout_blueprint::operator=(ft_loadout_blueprint &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_loadout_blueprint::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_loadout_id = other._loadout_id;
    game_data_catalog_copy_loadout_vector(other._entries, this->_entries);
    other._loadout_id = 0;
    other._entries.clear();
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}


int ft_loadout_blueprint::get_loadout_id() const noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    identifier = this->_loadout_id;
    game_data_catalog_unlock_guard(guard);
    return (identifier);
}

void ft_loadout_blueprint::set_loadout_id(int loadout_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_loadout_id = loadout_id;
    game_data_catalog_unlock_guard(guard);
    return ;
}

ft_vector<ft_loadout_entry> &ft_loadout_blueprint::get_entries() noexcept
{
    return (this->_entries);
}

const ft_vector<ft_loadout_entry> &ft_loadout_blueprint::get_entries() const noexcept
{
    return (this->_entries);
}

void ft_loadout_blueprint::set_entries(const ft_vector<ft_loadout_entry> &entries) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    game_data_catalog_copy_loadout_vector(entries, this->_entries);
    game_data_catalog_unlock_guard(guard);
    return ;
}

int ft_data_catalog::lock_pair(const ft_data_catalog &first,
        const ft_data_catalog &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_data_catalog *ordered_first;
    const ft_data_catalog *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_data_catalog *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_data_catalog_sleep_backoff();
    }
}


ft_data_catalog::ft_data_catalog() noexcept
{
    return ;
}

ft_data_catalog::~ft_data_catalog() noexcept
{
    return ;
}

ft_data_catalog::ft_data_catalog(const ft_data_catalog &other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_item_definitions = other._item_definitions;
    this->_recipes = other._recipes;
    this->_loadouts = other._loadouts;
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_data_catalog &ft_data_catalog::operator=(const ft_data_catalog &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_data_catalog::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_item_definitions = other._item_definitions;
    this->_recipes = other._recipes;
    this->_loadouts = other._loadouts;
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}

ft_data_catalog::ft_data_catalog(ft_data_catalog &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        return ;
    }
    this->_item_definitions = other._item_definitions;
    this->_recipes = other._recipes;
    this->_loadouts = other._loadouts;
    other._item_definitions.clear();
    other._recipes.clear();
    other._loadouts.clear();
    game_data_catalog_unlock_guard(other_guard);
    return ;
}

ft_data_catalog &ft_data_catalog::operator=(ft_data_catalog &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_data_catalog::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    this->_item_definitions = other._item_definitions;
    this->_recipes = other._recipes;
    this->_loadouts = other._loadouts;
    other._item_definitions.clear();
    other._recipes.clear();
    other._loadouts.clear();
    game_data_catalog_unlock_guard(this_guard);
    game_data_catalog_unlock_guard(other_guard);
    return (*this);
}

ft_map<int, ft_item_definition> &ft_data_catalog::get_item_definitions() noexcept
{
    return (this->_item_definitions);
}

const ft_map<int, ft_item_definition> &ft_data_catalog::get_item_definitions() const noexcept
{
    return (this->_item_definitions);
}

ft_map<int, ft_recipe_blueprint> &ft_data_catalog::get_recipes() noexcept
{
    return (this->_recipes);
}

const ft_map<int, ft_recipe_blueprint> &ft_data_catalog::get_recipes() const noexcept
{
    return (this->_recipes);
}

ft_map<int, ft_loadout_blueprint> &ft_data_catalog::get_loadouts() noexcept
{
    return (this->_loadouts);
}

const ft_map<int, ft_loadout_blueprint> &ft_data_catalog::get_loadouts() const noexcept
{
    return (this->_loadouts);
}

int ft_data_catalog::register_item_definition(const ft_item_definition &definition) noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    identifier = definition.get_item_id();
    this->_item_definitions.insert(identifier, definition);
    game_data_catalog_unlock_guard(guard);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::register_recipe(const ft_recipe_blueprint &recipe) noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    identifier = recipe.get_recipe_id();
    this->_recipes.insert(identifier, recipe);
    game_data_catalog_unlock_guard(guard);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::register_loadout(const ft_loadout_blueprint &loadout) noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    identifier = loadout.get_loadout_id();
    this->_loadouts.insert(identifier, loadout);
    game_data_catalog_unlock_guard(guard);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::fetch_item_definition(int item_id, ft_item_definition &definition) const noexcept
{
    const ft_data_catalog *self;
    const Pair<int, ft_item_definition> *entry;

    self = this;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    entry = self->_item_definitions.find(item_id);
    if (entry == self->_item_definitions.end())
    {
        return (FT_ERR_NOT_FOUND);
    }
    definition = entry->value;
    game_data_catalog_unlock_guard(guard);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::fetch_recipe(int recipe_id, ft_recipe_blueprint &recipe) const noexcept
{
    const ft_data_catalog *self;
    const Pair<int, ft_recipe_blueprint> *entry;

    self = this;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    entry = self->_recipes.find(recipe_id);
    if (entry == self->_recipes.end())
    {
        return (FT_ERR_NOT_FOUND);
    }
    recipe = entry->value;
    game_data_catalog_unlock_guard(guard);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::fetch_loadout(int loadout_id, ft_loadout_blueprint &loadout) const noexcept
{
    const ft_data_catalog *self;
    const Pair<int, ft_loadout_blueprint> *entry;

    self = this;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        return (guard.get_error());
    }
    entry = self->_loadouts.find(loadout_id);
    if (entry == self->_loadouts.end())
    {
        return (FT_ERR_NOT_FOUND);
    }
    loadout = entry->value;
    game_data_catalog_unlock_guard(guard);
    return (FT_ERR_SUCCESS);
}
