#include "game_crafting.hpp"
#include <new>

thread_local int ft_crafting_ingredient::_last_error = FT_ERR_SUCCESS;
thread_local int ft_crafting::_last_error = FT_ERR_SUCCESS;

void ft_crafting_ingredient::set_error(int error_code) const noexcept
{
    ft_crafting_ingredient::_last_error = error_code;
    return ;
}

int ft_crafting_ingredient::get_error() const noexcept
{
    return (ft_crafting_ingredient::_last_error);
}

const char *ft_crafting_ingredient::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

void ft_crafting::set_error(int error_code) const noexcept
{
    ft_crafting::_last_error = error_code;
    return ;
}

int ft_crafting::get_error() const noexcept
{
    return (ft_crafting::_last_error);
}

const char *ft_crafting::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

ft_crafting_ingredient::ft_crafting_ingredient() noexcept
    : _item_id(0), _count(0), _rarity(-1), _mutex(ft_nullptr)
{
    return ;
}

ft_crafting_ingredient::~ft_crafting_ingredient() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

ft_crafting::ft_crafting() noexcept
    : _recipes(), _mutex(ft_nullptr)
{
    return ;
}

ft_crafting::~ft_crafting() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

int ft_crafting_ingredient::initialize(
    const ft_crafting_ingredient &other) noexcept
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    int lock_error;

    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = false;
    other_lock_acquired = false;
    lock_error = this->lock_internal(&this_lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_item_id = other._item_id;
    this->_count = other._count;
    this->_rarity = other._rarity;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_crafting_ingredient::initialize(ft_crafting_ingredient &&other) noexcept
{
    return (this->initialize(static_cast<const ft_crafting_ingredient &>(other)));
}

int ft_crafting_ingredient::initialize(int item_id, int count, int rarity) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_item_id = item_id;
    this->_count = count;
    this->_rarity = rarity;
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_crafting_ingredient::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_crafting_ingredient::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    if (this->_mutex == ft_nullptr)
        return ;
    (void)this->_mutex->unlock();
    return ;
}

int ft_crafting_ingredient::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_crafting_ingredient::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *mutex_pointer;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_error != FT_ERR_SUCCESS && destroy_error != FT_ERR_INVALID_STATE)
    {
        this->set_error(destroy_error);
        return (destroy_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

bool ft_crafting_ingredient::is_thread_safe() const noexcept
{
    bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_crafting_ingredient::get_item_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int item_id;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    item_id = this->_item_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (item_id);
}

void ft_crafting_ingredient::set_item_id(int item_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_item_id = item_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_crafting_ingredient::get_count() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int count;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    count = this->_count;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (count);
}

void ft_crafting_ingredient::set_count(int count) noexcept
{
    bool lock_acquired;
    int lock_error;

    if (count < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_count = count;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_crafting_ingredient::get_rarity() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int rarity;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (-1);
    }
    rarity = this->_rarity;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (rarity);
}

void ft_crafting_ingredient::set_rarity(int rarity) noexcept
{
    bool lock_acquired;
    int lock_error;

    if (rarity < -1)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_rarity = rarity;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_crafting::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_crafting::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    if (this->_mutex == ft_nullptr)
        return ;
    (void)this->_mutex->unlock();
    return ;
}

int ft_crafting::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_crafting::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *mutex_pointer;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_error != FT_ERR_SUCCESS && destroy_error != FT_ERR_INVALID_STATE)
    {
        this->set_error(destroy_error);
        return (destroy_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

bool ft_crafting::is_thread_safe() const noexcept
{
    bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

ft_map<int, ft_vector<ft_crafting_ingredient>> &ft_crafting::get_recipes() noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_recipes);
}

const ft_map<int, ft_vector<ft_crafting_ingredient>> &ft_crafting::get_recipes() const noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_recipes);
}

int ft_crafting::register_recipe(int recipe_id,
    ft_vector<ft_crafting_ingredient> &&ingredients) noexcept
{
    ft_vector<ft_crafting_ingredient> copied_ingredients;
    size_t ingredient_index;
    size_t ingredient_count;
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    ingredient_index = 0;
    ingredient_count = ingredients.size();
    while (ingredient_index < ingredient_count)
    {
        copied_ingredients.push_back(ingredients[ingredient_index]);
        if (ft_vector<ft_crafting_ingredient>::last_operation_error() != FT_ERR_SUCCESS)
        {
            int error_code = ft_vector<ft_crafting_ingredient>::last_operation_error();
            this->unlock_internal(lock_acquired);
            this->set_error(error_code);
            return (error_code);
        }
        ingredient_index++;
    }
    this->_recipes.insert(recipe_id, copied_ingredients);
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_crafting::craft_item(ft_inventory &inventory, int recipe_id,
    const ft_sharedptr<ft_item> &result) noexcept
{
    Pair<int, ft_vector<ft_crafting_ingredient>> *recipe_entry;
    ft_vector<ft_crafting_ingredient> *ingredients;
    size_t ingredient_index;
    bool lock_acquired;
    int lock_error;

    if (!result)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    recipe_entry = this->_recipes.find(recipe_id);
    if (recipe_entry == this->_recipes.end())
    {
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
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
        {
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
        }
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
    this->unlock_internal(lock_acquired);
    int result_error = inventory.add_item(result);
    this->set_error(result_error);
    return (result_error);
}
