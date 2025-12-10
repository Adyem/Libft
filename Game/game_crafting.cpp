#include "game_crafting.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_crafting_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_crafting_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_crafting_ingredient::lock_pair(const ft_crafting_ingredient &first,
        const ft_crafting_ingredient &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_crafting_ingredient *ordered_first;
    const ft_crafting_ingredient *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_crafting_ingredient *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
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
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_crafting_sleep_backoff();
    }
}

ft_crafting_ingredient::ft_crafting_ingredient() noexcept
    : _item_id(0), _count(0), _rarity(-1), _error_code(FT_ER_SUCCESSS), _mutex()
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

ft_crafting_ingredient::ft_crafting_ingredient(int item_id, int count, int rarity) noexcept
    : _item_id(item_id), _count(count), _rarity(rarity), _error_code(FT_ER_SUCCESSS), _mutex()
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

ft_crafting_ingredient::ft_crafting_ingredient(const ft_crafting_ingredient &other) noexcept
    : _item_id(0), _count(0), _rarity(-1), _error_code(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_crafting_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_item_id = other._item_id;
    this->_count = other._count;
    this->_rarity = other._rarity;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_crafting_restore_errno(other_guard, entry_errno);
    return ;
}

ft_crafting_ingredient &ft_crafting_ingredient::operator=(const ft_crafting_ingredient &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_crafting_ingredient::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_item_id = other._item_id;
    this->_count = other._count;
    this->_rarity = other._rarity;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_crafting_restore_errno(this_guard, entry_errno);
    game_crafting_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_crafting_ingredient::ft_crafting_ingredient(ft_crafting_ingredient &&other) noexcept
    : _item_id(0), _count(0), _rarity(-1), _error_code(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_crafting_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_item_id = other._item_id;
    this->_count = other._count;
    this->_rarity = other._rarity;
    this->_error_code = other._error_code;
    other._item_id = 0;
    other._count = 0;
    other._rarity = -1;
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(other_guard, entry_errno);
    return ;
}

ft_crafting_ingredient &ft_crafting_ingredient::operator=(ft_crafting_ingredient &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_crafting_ingredient::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_item_id = other._item_id;
    this->_count = other._count;
    this->_rarity = other._rarity;
    this->_error_code = other._error_code;
    other._item_id = 0;
    other._count = 0;
    other._rarity = -1;
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(this_guard, entry_errno);
    game_crafting_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_crafting_ingredient::get_item_id() const noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_crafting_ingredient *>(this)->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (0);
    }
    identifier = this->_item_id;
    const_cast<ft_crafting_ingredient *>(this)->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return (identifier);
}

void ft_crafting_ingredient::set_item_id(int item_id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return ;
    }
    this->_item_id = item_id;
    this->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return ;
}

int ft_crafting_ingredient::get_count() const noexcept
{
    int entry_errno;
    int count_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_crafting_ingredient *>(this)->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (0);
    }
    count_value = this->_count;
    const_cast<ft_crafting_ingredient *>(this)->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return (count_value);
}

void ft_crafting_ingredient::set_count(int count) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return ;
    }
    if (count < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_crafting_restore_errno(guard, entry_errno);
        return ;
    }
    this->_count = count;
    this->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return ;
}

int ft_crafting_ingredient::get_rarity() const noexcept
{
    int entry_errno;
    int rarity_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_crafting_ingredient *>(this)->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (-1);
    }
    rarity_value = this->_rarity;
    const_cast<ft_crafting_ingredient *>(this)->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return (rarity_value);
}

void ft_crafting_ingredient::set_rarity(int rarity) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return ;
    }
    if (rarity < -1)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_crafting_restore_errno(guard, entry_errno);
        return ;
    }
    this->_rarity = rarity;
    this->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return ;
}

int ft_crafting_ingredient::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_crafting_ingredient *>(this)->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_crafting_ingredient *>(this)->set_error(error_code);
    game_crafting_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_crafting_ingredient::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_crafting_ingredient *>(this)->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_crafting_ingredient *>(this)->set_error(error_code);
    game_crafting_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_crafting_ingredient::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int ft_crafting::lock_pair(const ft_crafting &first, const ft_crafting &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_crafting *ordered_first;
    const ft_crafting *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_crafting *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
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
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_crafting_sleep_backoff();
    }
}

ft_crafting::ft_crafting() noexcept
    : _recipes(), _error_code(FT_ER_SUCCESSS), _mutex()
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

ft_crafting::ft_crafting(const ft_crafting &other) noexcept
    : _recipes(), _error_code(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;
    const Pair<int, ft_vector<ft_crafting_ingredient>> *entry;
    const Pair<int, ft_vector<ft_crafting_ingredient>> *end;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_crafting_restore_errno(other_guard, entry_errno);
        return ;
    }
    entry = other._recipes.end() - other._recipes.size();
    end = other._recipes.end();
    while (entry != end)
    {
        ft_vector<ft_crafting_ingredient> ingredients;
        size_t ingredient_index;

        ingredient_index = 0;
        while (ingredient_index < entry->value.size())
        {
            ingredients.push_back(entry->value[ingredient_index]);
            if (ingredients.get_error() != FT_ER_SUCCESSS)
            {
                this->set_error(ingredients.get_error());
                game_crafting_restore_errno(other_guard, entry_errno);
                return ;
            }
            ++ingredient_index;
        }
        this->_recipes.insert(entry->key, ft_move(ingredients));
        if (this->_recipes.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_recipes.get_error());
            game_crafting_restore_errno(other_guard, entry_errno);
            return ;
        }
        ++entry;
    }
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_crafting_restore_errno(other_guard, entry_errno);
    return ;
}

ft_crafting &ft_crafting::operator=(const ft_crafting &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_crafting::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_recipes.clear();
    const Pair<int, ft_vector<ft_crafting_ingredient>> *entry = other._recipes.end()
        - other._recipes.size();
    const Pair<int, ft_vector<ft_crafting_ingredient>> *end = other._recipes.end();
    while (entry != end)
    {
        ft_vector<ft_crafting_ingredient> ingredients;
        size_t ingredient_index;

        ingredient_index = 0;
        while (ingredient_index < entry->value.size())
        {
            ingredients.push_back(entry->value[ingredient_index]);
            if (ingredients.get_error() != FT_ER_SUCCESSS)
            {
                this->set_error(ingredients.get_error());
                game_crafting_restore_errno(this_guard, entry_errno);
                game_crafting_restore_errno(other_guard, entry_errno);
                return (*this);
            }
            ++ingredient_index;
        }
        this->_recipes.insert(entry->key, ft_move(ingredients));
        if (this->_recipes.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_recipes.get_error());
            game_crafting_restore_errno(this_guard, entry_errno);
            game_crafting_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        ++entry;
    }
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_crafting_restore_errno(this_guard, entry_errno);
    game_crafting_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_crafting::ft_crafting(ft_crafting &&other) noexcept
    : _recipes(), _error_code(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_crafting_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_recipes = ft_move(other._recipes);
    this->_error_code = other._error_code;
    if (this->_recipes.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_recipes.get_error());
    else
        this->set_error(this->_error_code);
    other._recipes.clear();
    other._error_code = FT_ER_SUCCESSS;
    other.set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(other_guard, entry_errno);
    return ;
}

ft_crafting &ft_crafting::operator=(ft_crafting &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_crafting::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_recipes = ft_move(other._recipes);
    this->_error_code = other._error_code;
    if (this->_recipes.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_recipes.get_error());
    else
        this->set_error(this->_error_code);
    other._recipes.clear();
    other._error_code = FT_ER_SUCCESSS;
    other.set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(this_guard, entry_errno);
    game_crafting_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_map<int, ft_vector<ft_crafting_ingredient>> &ft_crafting::get_recipes() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (this->_recipes);
    }
    this->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return (this->_recipes);
}

const ft_map<int, ft_vector<ft_crafting_ingredient>> &ft_crafting::get_recipes() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_crafting *>(this)->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (this->_recipes);
    }
    const_cast<ft_crafting *>(this)->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return (this->_recipes);
}

int ft_crafting::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_crafting *>(this)->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_crafting *>(this)->set_error(error_code);
    game_crafting_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_crafting::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_crafting *>(this)->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_crafting *>(this)->set_error(error_code);
    game_crafting_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_crafting::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int ft_crafting::register_recipe(int recipe_id,
        ft_vector<ft_crafting_ingredient> &&ingredients) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->set_error(FT_ER_SUCCESSS);
    this->_recipes.insert(recipe_id, ft_move(ingredients));
    if (this->_recipes.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_recipes.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    game_crafting_restore_errno(guard, entry_errno);
    return (FT_ER_SUCCESSS);
}

int ft_crafting::craft_item(ft_inventory &inventory, int recipe_id,
        const ft_sharedptr<ft_item> &result) noexcept
{
    int entry_errno;
    Pair<int, ft_vector<ft_crafting_ingredient>> *recipe_entry;
    ft_vector<ft_crafting_ingredient> *ingredients;
    size_t index;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->set_error(FT_ER_SUCCESSS);
    if (!result)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        game_crafting_restore_errno(guard, entry_errno);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    if (result.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(result.get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (result->get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(result->get_error());
        game_crafting_restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    recipe_entry = this->_recipes.find(recipe_id);
    if (recipe_entry == this->_recipes.end())
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        game_crafting_restore_errno(guard, entry_errno);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    ingredients = &recipe_entry->value;
    index = 0;
    while (index < ingredients->size())
    {
        ft_crafting_ingredient &ingredient = (*ingredients)[index];
        int ingredient_item_id;
        int ingredient_count;
        int ingredient_rarity;
        int have_count;
        const ft_map<int, ft_sharedptr<ft_item> > &items = inventory.get_items();
        const Pair<int, ft_sharedptr<ft_item> > *item_ptr;
        const Pair<int, ft_sharedptr<ft_item> > *item_end;
        int inventory_error;

        ingredient_item_id = ingredient.get_item_id();
        if (ingredient.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(ingredient.get_error());
            game_crafting_restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        ingredient_count = ingredient.get_count();
        if (ingredient.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(ingredient.get_error());
            game_crafting_restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        ingredient_rarity = ingredient.get_rarity();
        if (ingredient.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(ingredient.get_error());
            game_crafting_restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        have_count = 0;
        inventory_error = inventory.get_error();
        if (inventory_error != FT_ER_SUCCESSS)
        {
            this->set_error(inventory_error);
            game_crafting_restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        item_ptr = items.end() - items.size();
        item_end = items.end();
        while (item_ptr != item_end)
        {
            if (item_ptr->value)
            {
                if (item_ptr->value.get_error() != FT_ER_SUCCESSS)
                {
                    this->set_error(item_ptr->value.get_error());
                    game_crafting_restore_errno(guard, entry_errno);
                    return (this->_error_code);
                }
                if (item_ptr->value->get_error() != FT_ER_SUCCESSS)
                {
                    this->set_error(item_ptr->value->get_error());
                    game_crafting_restore_errno(guard, entry_errno);
                    return (this->_error_code);
                }
                if (item_ptr->value->get_item_id() == ingredient_item_id)
                {
                    if (ingredient_rarity == -1
                            || item_ptr->value->get_rarity() == ingredient_rarity)
                        have_count += item_ptr->value->get_stack_size();
                }
            }
            ++item_ptr;
        }
        if (have_count < ingredient_count)
        {
            this->set_error(FT_ERR_GAME_GENERAL_ERROR);
            game_crafting_restore_errno(guard, entry_errno);
            return (FT_ERR_GAME_GENERAL_ERROR);
        }
        ++index;
    }
    index = 0;
    while (index < ingredients->size())
    {
        ft_crafting_ingredient &ingredient = (*ingredients)[index];
        int ingredient_item_id;
        int ingredient_count;
        int ingredient_rarity;
        int remaining;
        ft_map<int, ft_sharedptr<ft_item> > &items = inventory.get_items();
        Pair<int, ft_sharedptr<ft_item> > *item_ptr;
        Pair<int, ft_sharedptr<ft_item> > *item_end;
        int inventory_error;

        ingredient_item_id = ingredient.get_item_id();
        if (ingredient.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(ingredient.get_error());
            game_crafting_restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        ingredient_count = ingredient.get_count();
        if (ingredient.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(ingredient.get_error());
            game_crafting_restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        ingredient_rarity = ingredient.get_rarity();
        if (ingredient.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(ingredient.get_error());
            game_crafting_restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        remaining = ingredient_count;
        inventory_error = inventory.get_error();
        if (inventory_error != FT_ER_SUCCESSS)
        {
            this->set_error(inventory_error);
            game_crafting_restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        item_ptr = items.end() - items.size();
        item_end = items.end();
        while (item_ptr != item_end && remaining > 0)
        {
            if (item_ptr->value)
            {
                if (item_ptr->value.get_error() != FT_ER_SUCCESSS)
                {
                    this->set_error(item_ptr->value.get_error());
                    game_crafting_restore_errno(guard, entry_errno);
                    return (this->_error_code);
                }
                if (item_ptr->value->get_error() != FT_ER_SUCCESSS)
                {
                    this->set_error(item_ptr->value->get_error());
                    game_crafting_restore_errno(guard, entry_errno);
                    return (this->_error_code);
                }
                if (item_ptr->value->get_item_id() == ingredient_item_id)
                {
                    if (ingredient_rarity == -1
                            || item_ptr->value->get_rarity() == ingredient_rarity)
                    {
                        int remove;

                        remove = item_ptr->value->get_stack_size();
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
    int add_error;

    add_error = inventory.add_item(result);
    if (add_error != FT_ER_SUCCESSS)
    {
        this->set_error(add_error);
        game_crafting_restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->set_error(FT_ER_SUCCESSS);
    game_crafting_restore_errno(guard, entry_errno);
    return (FT_ER_SUCCESSS);
}

