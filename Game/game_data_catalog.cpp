#include "game_data_catalog.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_item_definition::_last_error = FT_ERR_SUCCESS;

void ft_item_definition::set_error(int error_code) const noexcept
{
    ft_item_definition::_last_error = error_code;
    return ;
}

int ft_item_definition::get_error() const noexcept
{
    return (ft_item_definition::_last_error);
}

const char *ft_item_definition::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

thread_local int ft_loadout_entry::_last_error = FT_ERR_SUCCESS;

void ft_loadout_entry::set_error(int error_code) const noexcept
{
    ft_loadout_entry::_last_error = error_code;
    return ;
}

int ft_loadout_entry::get_error() const noexcept
{
    return (ft_loadout_entry::_last_error);
}

const char *ft_loadout_entry::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

thread_local int ft_loadout_blueprint::_last_error = FT_ERR_SUCCESS;

void ft_loadout_blueprint::set_error(int error_code) const noexcept
{
    ft_loadout_blueprint::_last_error = error_code;
    return ;
}

int ft_loadout_blueprint::get_error() const noexcept
{
    return (ft_loadout_blueprint::_last_error);
}

const char *ft_loadout_blueprint::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

thread_local int ft_recipe_blueprint::_last_error = FT_ERR_SUCCESS;

void ft_recipe_blueprint::set_error(int error_code) const noexcept
{
    ft_recipe_blueprint::_last_error = error_code;
    return ;
}

int ft_recipe_blueprint::get_error() const noexcept
{
    return (ft_recipe_blueprint::_last_error);
}

const char *ft_recipe_blueprint::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

static void game_data_catalog_sleep_backoff()
{
    pt_thread_sleep(1);
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
        bool *first_locked,
        bool *second_locked)
{
    const ft_item_definition *ordered_first;
    const ft_item_definition *ordered_second;
    int lock_error;
    bool swapped;

    if (first_locked != ft_nullptr)
        *first_locked = false;
    if (second_locked != ft_nullptr)
        *second_locked = false;
    if (&first == &second)
        return (first.lock_internal(first_locked));
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
        bool lower_locked;
        bool upper_locked;

        lower_locked = false;
        upper_locked = false;
        lock_error = ordered_first->lock_internal(&lower_locked);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
        lock_error = ordered_second->lock_internal(&upper_locked);
        if (lock_error == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                if (first_locked != ft_nullptr)
                    *first_locked = lower_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = upper_locked;
            }
            else
            {
                if (first_locked != ft_nullptr)
                    *first_locked = upper_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = lower_locked;
            }
            return (FT_ERR_SUCCESS);
        }
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_internal(lower_locked);
            return (lock_error);
        }
        ordered_first->unlock_internal(lower_locked);
        game_data_catalog_sleep_backoff();
    }
}

ft_item_definition::ft_item_definition() noexcept
    : _item_id(0), _rarity(0), _max_stack(0), _width(0), _height(0), _weight(0),
      _slot_requirement(0), _mutex(ft_nullptr),
      _initialized_state(ft_item_definition::_state_uninitialized)
{
    return ;
}

ft_item_definition::~ft_item_definition() noexcept
{
    if (this->_initialized_state == ft_item_definition::_state_initialized)
        (void)this->destroy();
    (void)this->disable_thread_safety();
    return ;
}

int ft_item_definition::enable_thread_safety() noexcept
{
    int initialize_error;
    pt_recursive_mutex *new_mutex;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = new_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = new_mutex;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_item_definition::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_item_definition::is_thread_safe() const noexcept
{
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

void ft_item_definition::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_item_definition lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_item_definition::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_item_definition::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_item_definition::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_item_definition::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_item_definition::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    (void)this->_mutex->unlock();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item_definition::initialize() noexcept
{
    if (this->_initialized_state == ft_item_definition::_state_initialized)
    {
        this->abort_lifecycle_error("ft_item_definition::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_item_id = 0;
    this->_rarity = 0;
    this->_max_stack = 0;
    this->_width = 0;
    this->_height = 0;
    this->_weight = 0;
    this->_slot_requirement = 0;
    this->_initialized_state = ft_item_definition::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_item_definition::initialize(int item_id, int rarity, int max_stack,
    int width, int height, int weight, int slot_requirement) noexcept
{
    if (this->_initialized_state == ft_item_definition::_state_initialized)
    {
        this->abort_lifecycle_error("ft_item_definition::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_item_id = item_id;
    this->_rarity = rarity;
    this->_max_stack = max_stack;
    this->_width = width;
    this->_height = height;
    this->_weight = weight;
    this->_slot_requirement = slot_requirement;
    this->_initialized_state = ft_item_definition::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_item_definition::initialize(const ft_item_definition &other) noexcept
{
    bool other_locked;
    int lock_error;

    if (other._initialized_state != ft_item_definition::_state_initialized)
    {
        other.abort_lifecycle_error("ft_item_definition::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_item_definition::_state_initialized)
    {
        this->abort_lifecycle_error("ft_item_definition::initialize(copy)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_item_definition::_state_destroyed;
        return (lock_error);
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_max_stack = other._max_stack;
    this->_width = other._width;
    this->_height = other._height;
    this->_weight = other._weight;
    this->_slot_requirement = other._slot_requirement;
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_item_definition::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_item_definition::initialize(ft_item_definition &&other) noexcept
{
    bool other_locked;
    int lock_error;

    if (other._initialized_state != ft_item_definition::_state_initialized)
    {
        other.abort_lifecycle_error("ft_item_definition::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_item_definition::_state_initialized)
    {
        this->abort_lifecycle_error("ft_item_definition::initialize(move)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_item_definition::_state_destroyed;
        return (lock_error);
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
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_item_definition::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_item_definition::destroy() noexcept
{
    if (this->_initialized_state != ft_item_definition::_state_initialized)
    {
        this->_initialized_state = ft_item_definition::_state_destroyed;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_item_id = 0;
    this->_rarity = 0;
    this->_max_stack = 0;
    this->_width = 0;
    this->_height = 0;
    this->_weight = 0;
    this->_slot_requirement = 0;
    this->_initialized_state = ft_item_definition::_state_destroyed;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_item_definition::get_item_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;

    this->abort_if_not_initialized("ft_item_definition::get_item_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    identifier = this->_item_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (identifier);
}

void ft_item_definition::set_item_id(int item_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_item_definition::set_item_id");
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

int ft_item_definition::get_rarity() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int rarity_value;

    this->abort_if_not_initialized("ft_item_definition::get_rarity");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    rarity_value = this->_rarity;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (rarity_value);
}

void ft_item_definition::set_rarity(int rarity) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_item_definition::set_rarity");
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

int ft_item_definition::get_max_stack() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    this->abort_if_not_initialized("ft_item_definition::get_max_stack");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    value = this->_max_stack;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (value);
}

void ft_item_definition::set_max_stack(int max_stack) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_item_definition::set_max_stack");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_max_stack = max_stack;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item_definition::get_width() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int width_value;

    this->abort_if_not_initialized("ft_item_definition::get_width");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    width_value = this->_width;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (width_value);
}

void ft_item_definition::set_width(int width) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_item_definition::set_width");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_width = width;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}


int ft_item_definition::get_height() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int height_value;

    this->abort_if_not_initialized("ft_item_definition::get_height");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    height_value = this->_height;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (height_value);
}

void ft_item_definition::set_height(int height) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_item_definition::set_height");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_height = height;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item_definition::get_weight() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int weight_value;

    this->abort_if_not_initialized("ft_item_definition::get_weight");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    weight_value = this->_weight;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (weight_value);
}

void ft_item_definition::set_weight(int weight) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_item_definition::set_weight");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_weight = weight;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item_definition::get_slot_requirement() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int slot_value;

    this->abort_if_not_initialized("ft_item_definition::get_slot_requirement");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    slot_value = this->_slot_requirement;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (slot_value);
}

void ft_item_definition::set_slot_requirement(int slot_requirement) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_item_definition::set_slot_requirement");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_slot_requirement = slot_requirement;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}



int ft_recipe_blueprint::lock_pair(const ft_recipe_blueprint &first,
        const ft_recipe_blueprint &second,
        bool *first_locked,
        bool *second_locked)
{
    const ft_recipe_blueprint *ordered_first;
    const ft_recipe_blueprint *ordered_second;
    int lock_error;
    bool swapped;

    if (first_locked != ft_nullptr)
        *first_locked = false;
    if (second_locked != ft_nullptr)
        *second_locked = false;
    if (&first == &second)
        return (first.lock_internal(first_locked));
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
        bool lower_locked;
        bool upper_locked;

        lower_locked = false;
        upper_locked = false;
        lock_error = ordered_first->lock_internal(&lower_locked);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
        lock_error = ordered_second->lock_internal(&upper_locked);
        if (lock_error == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                if (first_locked != ft_nullptr)
                    *first_locked = lower_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = upper_locked;
            }
            else
            {
                if (first_locked != ft_nullptr)
                    *first_locked = upper_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = lower_locked;
            }
            return (FT_ERR_SUCCESS);
        }
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_internal(lower_locked);
            return (lock_error);
        }
        ordered_first->unlock_internal(lower_locked);
        game_data_catalog_sleep_backoff();
    }
}

ft_recipe_blueprint::ft_recipe_blueprint() noexcept
    : _recipe_id(0), _result_item_id(0), _mutex(ft_nullptr),
      _initialized_state(ft_recipe_blueprint::_state_uninitialized)
{
    return ;
}

ft_recipe_blueprint::~ft_recipe_blueprint() noexcept
{
    if (this->_initialized_state == ft_recipe_blueprint::_state_initialized)
        (void)this->destroy();
    (void)this->disable_thread_safety();
    return ;
}

int ft_recipe_blueprint::enable_thread_safety() noexcept
{
    int initialize_error;
    pt_recursive_mutex *new_mutex;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = new_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = new_mutex;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_recipe_blueprint::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_recipe_blueprint::is_thread_safe() const noexcept
{
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

void ft_recipe_blueprint::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_recipe_blueprint lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_recipe_blueprint::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_recipe_blueprint::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_recipe_blueprint::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_recipe_blueprint::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_recipe_blueprint::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    (void)this->_mutex->unlock();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_recipe_blueprint::initialize() noexcept
{
    int vector_initialize_error;

    if (this->_initialized_state == ft_recipe_blueprint::_state_initialized)
    {
        this->abort_lifecycle_error("ft_recipe_blueprint::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    vector_initialize_error = this->_ingredients.initialize();
    if (vector_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_recipe_blueprint::_state_destroyed;
        return (vector_initialize_error);
    }
    this->_recipe_id = 0;
    this->_result_item_id = 0;
    this->_ingredients.clear();
    this->_initialized_state = ft_recipe_blueprint::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_recipe_blueprint::initialize(int recipe_id, int result_item_id,
    const ft_vector<ft_crafting_ingredient> &ingredients) noexcept
{
    int vector_initialize_error;

    if (this->_initialized_state == ft_recipe_blueprint::_state_initialized)
    {
        this->abort_lifecycle_error("ft_recipe_blueprint::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    vector_initialize_error = this->_ingredients.initialize();
    if (vector_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_recipe_blueprint::_state_destroyed;
        return (vector_initialize_error);
    }
    this->_recipe_id = recipe_id;
    this->_result_item_id = result_item_id;
    game_data_catalog_copy_crafting_vector(ingredients, this->_ingredients);
    this->_initialized_state = ft_recipe_blueprint::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_recipe_blueprint::initialize(const ft_recipe_blueprint &other) noexcept
{
    bool other_locked;
    int lock_error;
    int vector_initialize_error;

    if (other._initialized_state != ft_recipe_blueprint::_state_initialized)
    {
        other.abort_lifecycle_error("ft_recipe_blueprint::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_recipe_blueprint::_state_initialized)
    {
        this->abort_lifecycle_error("ft_recipe_blueprint::initialize(copy)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    vector_initialize_error = this->_ingredients.initialize();
    if (vector_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_recipe_blueprint::_state_destroyed;
        return (vector_initialize_error);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->_ingredients.destroy();
        this->_initialized_state = ft_recipe_blueprint::_state_destroyed;
        return (lock_error);
    }
    this->_recipe_id = other._recipe_id;
    this->_result_item_id = other._result_item_id;
    game_data_catalog_copy_crafting_vector(other._ingredients, this->_ingredients);
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_recipe_blueprint::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_recipe_blueprint::initialize(ft_recipe_blueprint &&other) noexcept
{
    bool other_locked;
    int lock_error;
    int vector_initialize_error;

    if (other._initialized_state != ft_recipe_blueprint::_state_initialized)
    {
        other.abort_lifecycle_error("ft_recipe_blueprint::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_recipe_blueprint::_state_initialized)
    {
        this->abort_lifecycle_error("ft_recipe_blueprint::initialize(move)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    vector_initialize_error = this->_ingredients.initialize();
    if (vector_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_recipe_blueprint::_state_destroyed;
        return (vector_initialize_error);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->_ingredients.destroy();
        this->_initialized_state = ft_recipe_blueprint::_state_destroyed;
        return (lock_error);
    }
    this->_recipe_id = other._recipe_id;
    this->_result_item_id = other._result_item_id;
    game_data_catalog_copy_crafting_vector(other._ingredients, this->_ingredients);
    other._recipe_id = 0;
    other._result_item_id = 0;
    other._ingredients.clear();
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_recipe_blueprint::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_recipe_blueprint::destroy() noexcept
{
    if (this->_initialized_state != ft_recipe_blueprint::_state_initialized)
    {
        this->_initialized_state = ft_recipe_blueprint::_state_destroyed;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)this->_ingredients.destroy();
    this->_recipe_id = 0;
    this->_result_item_id = 0;
    this->_initialized_state = ft_recipe_blueprint::_state_destroyed;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}


int ft_recipe_blueprint::get_recipe_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;

    this->abort_if_not_initialized("ft_recipe_blueprint::get_recipe_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    identifier = this->_recipe_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (identifier);
}

void ft_recipe_blueprint::set_recipe_id(int recipe_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_recipe_blueprint::set_recipe_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_recipe_id = recipe_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_recipe_blueprint::get_result_item_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int item_id;

    this->abort_if_not_initialized("ft_recipe_blueprint::get_result_item_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    item_id = this->_result_item_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (item_id);
}

void ft_recipe_blueprint::set_result_item_id(int result_item_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_recipe_blueprint::set_result_item_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_result_item_id = result_item_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

ft_vector<ft_crafting_ingredient> &ft_recipe_blueprint::get_ingredients() noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_recipe_blueprint::get_ingredients");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_ingredients);
    }
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_ingredients);
}

const ft_vector<ft_crafting_ingredient> &ft_recipe_blueprint::get_ingredients() const noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_recipe_blueprint::get_ingredients const");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_ingredients);
    }
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_ingredients);
}

void ft_recipe_blueprint::set_ingredients(const ft_vector<ft_crafting_ingredient> &ingredients) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_recipe_blueprint::set_ingredients");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    game_data_catalog_copy_crafting_vector(ingredients, this->_ingredients);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}


int ft_loadout_entry::lock_pair(const ft_loadout_entry &first,
        const ft_loadout_entry &second,
        bool *first_locked,
        bool *second_locked)
{
    const ft_loadout_entry *ordered_first;
    const ft_loadout_entry *ordered_second;
    int lock_error;
    bool swapped;

    if (first_locked != ft_nullptr)
        *first_locked = false;
    if (second_locked != ft_nullptr)
        *second_locked = false;
    if (&first == &second)
        return (first.lock_internal(first_locked));
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
        bool lower_locked;
        bool upper_locked;

        lower_locked = false;
        upper_locked = false;
        lock_error = ordered_first->lock_internal(&lower_locked);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
        lock_error = ordered_second->lock_internal(&upper_locked);
        if (lock_error == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                if (first_locked != ft_nullptr)
                    *first_locked = lower_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = upper_locked;
            }
            else
            {
                if (first_locked != ft_nullptr)
                    *first_locked = upper_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = lower_locked;
            }
            return (FT_ERR_SUCCESS);
        }
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_internal(lower_locked);
            return (lock_error);
        }
        ordered_first->unlock_internal(lower_locked);
        game_data_catalog_sleep_backoff();
    }
}

ft_loadout_entry::ft_loadout_entry() noexcept
    : _slot(0), _item_id(0), _quantity(0), _mutex(ft_nullptr),
      _initialized_state(ft_loadout_entry::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_loadout_entry::~ft_loadout_entry() noexcept
{
    if (this->_initialized_state == ft_loadout_entry::_state_initialized)
        (void)this->destroy();
    (void)this->disable_thread_safety();
    return ;
}

int ft_loadout_entry::enable_thread_safety() noexcept
{
    int initialize_error;
    pt_recursive_mutex *new_mutex;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = new_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = new_mutex;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_loadout_entry::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_loadout_entry::is_thread_safe() const noexcept
{
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

void ft_loadout_entry::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_loadout_entry lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_loadout_entry::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_loadout_entry::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_loadout_entry::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_loadout_entry::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_loadout_entry::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    (void)this->_mutex->unlock();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_loadout_entry::initialize() noexcept
{
    if (this->_initialized_state == ft_loadout_entry::_state_initialized)
    {
        this->abort_lifecycle_error("ft_loadout_entry::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_slot = 0;
    this->_item_id = 0;
    this->_quantity = 0;
    this->_initialized_state = ft_loadout_entry::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_loadout_entry::initialize(int slot, int item_id, int quantity) noexcept
{
    if (this->_initialized_state == ft_loadout_entry::_state_initialized)
    {
        this->abort_lifecycle_error("ft_loadout_entry::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_slot = slot;
    this->_item_id = item_id;
    this->_quantity = quantity;
    this->_initialized_state = ft_loadout_entry::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_loadout_entry::initialize(const ft_loadout_entry &other) noexcept
{
    bool other_locked;
    int lock_error;
    if (other._initialized_state != ft_loadout_entry::_state_initialized)
    {
        other.abort_lifecycle_error("ft_loadout_entry::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_loadout_entry::_state_initialized)
    {
        this->abort_lifecycle_error("ft_loadout_entry::initialize(copy)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_loadout_entry::_state_destroyed;
        return (lock_error);
    }
    this->_slot = other._slot;
    this->_item_id = other._item_id;
    this->_quantity = other._quantity;
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_loadout_entry::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_loadout_entry::initialize(ft_loadout_entry &&other) noexcept
{
    bool other_locked;
    int lock_error;
    if (other._initialized_state != ft_loadout_entry::_state_initialized)
    {
        other.abort_lifecycle_error("ft_loadout_entry::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_loadout_entry::_state_initialized)
    {
        this->abort_lifecycle_error("ft_loadout_entry::initialize(move)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_loadout_entry::_state_destroyed;
        return (lock_error);
    }
    this->_slot = other._slot;
    this->_item_id = other._item_id;
    this->_quantity = other._quantity;
    other._slot = 0;
    other._item_id = 0;
    other._quantity = 0;
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_loadout_entry::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_loadout_entry::destroy() noexcept
{
    if (this->_initialized_state != ft_loadout_entry::_state_initialized)
    {
        this->_initialized_state = ft_loadout_entry::_state_destroyed;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_slot = 0;
    this->_item_id = 0;
    this->_quantity = 0;
    this->_initialized_state = ft_loadout_entry::_state_destroyed;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}


int ft_loadout_entry::get_slot() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int slot_value;

    this->abort_if_not_initialized("ft_loadout_entry::get_slot");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    slot_value = this->_slot;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (slot_value);
}

void ft_loadout_entry::set_slot(int slot) noexcept
{
    bool lock_acquired;
    int lock_error;
    this->abort_if_not_initialized("ft_loadout_entry::set_slot");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_slot = slot;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_loadout_entry::get_item_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int item_id;

    this->abort_if_not_initialized("ft_loadout_entry::get_item_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    item_id = this->_item_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (item_id);
}

void ft_loadout_entry::set_item_id(int item_id) noexcept
{
    bool lock_acquired;
    int lock_error;
    this->abort_if_not_initialized("ft_loadout_entry::set_item_id");
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

int ft_loadout_entry::get_quantity() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int quantity_value;

    this->abort_if_not_initialized("ft_loadout_entry::get_quantity");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    quantity_value = this->_quantity;
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (quantity_value);
}

void ft_loadout_entry::set_quantity(int quantity) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_loadout_entry::set_quantity");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_quantity = quantity;
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}


int ft_loadout_blueprint::lock_pair(const ft_loadout_blueprint &first,
        const ft_loadout_blueprint &second,
        bool *first_locked,
        bool *second_locked)
{
    const ft_loadout_blueprint *ordered_first;
    const ft_loadout_blueprint *ordered_second;
    int lock_error;
    bool swapped;

    if (first_locked != ft_nullptr)
        *first_locked = false;
    if (second_locked != ft_nullptr)
        *second_locked = false;
    if (&first == &second)
        return (first.lock_internal(first_locked));
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
        bool lower_locked;
        bool upper_locked;

        lower_locked = false;
        upper_locked = false;
        lock_error = ordered_first->lock_internal(&lower_locked);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
        lock_error = ordered_second->lock_internal(&upper_locked);
        if (lock_error == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                if (first_locked != ft_nullptr)
                    *first_locked = lower_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = upper_locked;
            }
            else
            {
                if (first_locked != ft_nullptr)
                    *first_locked = upper_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = lower_locked;
            }
            return (FT_ERR_SUCCESS);
        }
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_internal(lower_locked);
            return (lock_error);
        }
        ordered_first->unlock_internal(lower_locked);
        game_data_catalog_sleep_backoff();
    }
}

ft_loadout_blueprint::ft_loadout_blueprint() noexcept
    : _loadout_id(0), _mutex(ft_nullptr),
      _initialized_state(ft_loadout_blueprint::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_loadout_blueprint::~ft_loadout_blueprint() noexcept
{
    if (this->_initialized_state == ft_loadout_blueprint::_state_initialized)
        (void)this->destroy();
    (void)this->disable_thread_safety();
    return ;
}

int ft_loadout_blueprint::enable_thread_safety() noexcept
{
    int initialize_error;
    pt_recursive_mutex *new_mutex;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = new_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = new_mutex;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_loadout_blueprint::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_loadout_blueprint::is_thread_safe() const noexcept
{
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

void ft_loadout_blueprint::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_loadout_blueprint lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_loadout_blueprint::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_loadout_blueprint::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_loadout_blueprint::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_loadout_blueprint::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_loadout_blueprint::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    (void)this->_mutex->unlock();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_loadout_blueprint::initialize() noexcept
{
    int vector_initialize_error;

    if (this->_initialized_state == ft_loadout_blueprint::_state_initialized)
    {
        this->abort_lifecycle_error("ft_loadout_blueprint::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    vector_initialize_error = this->_entries.initialize();
    if (vector_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_loadout_blueprint::_state_destroyed;
        return (vector_initialize_error);
    }
    this->_loadout_id = 0;
    this->_entries.clear();
    this->_initialized_state = ft_loadout_blueprint::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_loadout_blueprint::initialize(int loadout_id,
    const ft_vector<ft_loadout_entry> &entries) noexcept
{
    int vector_initialize_error;

    if (this->_initialized_state == ft_loadout_blueprint::_state_initialized)
    {
        this->abort_lifecycle_error("ft_loadout_blueprint::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    vector_initialize_error = this->_entries.initialize();
    if (vector_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_loadout_blueprint::_state_destroyed;
        return (vector_initialize_error);
    }
    this->_loadout_id = loadout_id;
    game_data_catalog_copy_loadout_vector(entries, this->_entries);
    this->_initialized_state = ft_loadout_blueprint::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_loadout_blueprint::initialize(const ft_loadout_blueprint &other) noexcept
{
    bool other_locked;
    int lock_error;
    int vector_initialize_error;

    if (other._initialized_state != ft_loadout_blueprint::_state_initialized)
    {
        other.abort_lifecycle_error("ft_loadout_blueprint::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_loadout_blueprint::_state_initialized)
    {
        this->abort_lifecycle_error("ft_loadout_blueprint::initialize(copy)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    vector_initialize_error = this->_entries.initialize();
    if (vector_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_loadout_blueprint::_state_destroyed;
        return (vector_initialize_error);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->_entries.destroy();
        this->_initialized_state = ft_loadout_blueprint::_state_destroyed;
        return (lock_error);
    }
    this->_loadout_id = other._loadout_id;
    game_data_catalog_copy_loadout_vector(other._entries, this->_entries);
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_loadout_blueprint::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_loadout_blueprint::initialize(ft_loadout_blueprint &&other) noexcept
{
    bool other_locked;
    int lock_error;
    int vector_initialize_error;

    if (other._initialized_state != ft_loadout_blueprint::_state_initialized)
    {
        other.abort_lifecycle_error("ft_loadout_blueprint::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_loadout_blueprint::_state_initialized)
    {
        this->abort_lifecycle_error("ft_loadout_blueprint::initialize(move)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    vector_initialize_error = this->_entries.initialize();
    if (vector_initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_loadout_blueprint::_state_destroyed;
        return (vector_initialize_error);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->_entries.destroy();
        this->_initialized_state = ft_loadout_blueprint::_state_destroyed;
        return (lock_error);
    }
    this->_loadout_id = other._loadout_id;
    game_data_catalog_copy_loadout_vector(other._entries, this->_entries);
    other._loadout_id = 0;
    other._entries.clear();
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_loadout_blueprint::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_loadout_blueprint::destroy() noexcept
{
    if (this->_initialized_state != ft_loadout_blueprint::_state_initialized)
    {
        this->_initialized_state = ft_loadout_blueprint::_state_destroyed;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)this->_entries.destroy();
    this->_loadout_id = 0;
    this->_initialized_state = ft_loadout_blueprint::_state_destroyed;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}


int ft_loadout_blueprint::get_loadout_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;

    this->abort_if_not_initialized("ft_loadout_blueprint::get_loadout_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    identifier = this->_loadout_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (identifier);
}

void ft_loadout_blueprint::set_loadout_id(int loadout_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_loadout_blueprint::set_loadout_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_loadout_id = loadout_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

ft_vector<ft_loadout_entry> &ft_loadout_blueprint::get_entries() noexcept
{
    this->abort_if_not_initialized("ft_loadout_blueprint::get_entries");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_entries);
}

const ft_vector<ft_loadout_entry> &ft_loadout_blueprint::get_entries() const noexcept
{
    this->abort_if_not_initialized("ft_loadout_blueprint::get_entries const");
    const_cast<ft_loadout_blueprint *>(this)->set_error(FT_ERR_SUCCESS);
    return (this->_entries);
}

void ft_loadout_blueprint::set_entries(const ft_vector<ft_loadout_entry> &entries) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_loadout_blueprint::set_entries");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    game_data_catalog_copy_loadout_vector(entries, this->_entries);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

thread_local int ft_data_catalog::_last_error = FT_ERR_SUCCESS;

void ft_data_catalog::set_error(int error_code) const noexcept
{
    ft_data_catalog::_last_error = error_code;
    return ;
}

int ft_data_catalog::get_error() const noexcept
{
    return (ft_data_catalog::_last_error);
}

const char *ft_data_catalog::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

int ft_data_catalog::lock_pair(const ft_data_catalog &first,
        const ft_data_catalog &second,
        bool *first_locked,
        bool *second_locked)
{
    const ft_data_catalog *ordered_first;
    const ft_data_catalog *ordered_second;
    int lock_error;
    bool swapped;

    if (first_locked != ft_nullptr)
        *first_locked = false;
    if (second_locked != ft_nullptr)
        *second_locked = false;
    if (&first == &second)
        return (first.lock_internal(first_locked));
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
        bool lower_locked;
        bool upper_locked;

        lower_locked = false;
        upper_locked = false;
        lock_error = ordered_first->lock_internal(&lower_locked);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
        lock_error = ordered_second->lock_internal(&upper_locked);
        if (lock_error == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                if (first_locked != ft_nullptr)
                    *first_locked = lower_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = upper_locked;
            }
            else
            {
                if (first_locked != ft_nullptr)
                    *first_locked = upper_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = lower_locked;
            }
            return (FT_ERR_SUCCESS);
        }
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_internal(lower_locked);
            return (lock_error);
        }
        ordered_first->unlock_internal(lower_locked);
        game_data_catalog_sleep_backoff();
    }
}


ft_data_catalog::ft_data_catalog() noexcept
    : _item_definitions(), _recipes(), _loadouts(), _mutex(ft_nullptr),
      _initialized_state(ft_data_catalog::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_data_catalog::~ft_data_catalog() noexcept
{
    if (this->_initialized_state == ft_data_catalog::_state_initialized)
        (void)this->destroy();
    (void)this->disable_thread_safety();
    return ;
}

int ft_data_catalog::enable_thread_safety() noexcept
{
    int initialize_error;
    pt_recursive_mutex *new_mutex;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = new_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = new_mutex;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_data_catalog::is_thread_safe() const noexcept
{
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

void ft_data_catalog::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_data_catalog lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_data_catalog::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_data_catalog::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_data_catalog::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_data_catalog::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_data_catalog::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    (void)this->_mutex->unlock();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_data_catalog::initialize() noexcept
{
    int init_error;
    if (this->_initialized_state == ft_data_catalog::_state_initialized)
    {
        this->abort_lifecycle_error("ft_data_catalog::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    init_error = this->_item_definitions.initialize();
    if (init_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_data_catalog::_state_destroyed;
        return (init_error);
    }
    init_error = this->_recipes.initialize();
    if (init_error != FT_ERR_SUCCESS)
    {
        (void)this->_item_definitions.destroy();
        this->_initialized_state = ft_data_catalog::_state_destroyed;
        return (init_error);
    }
    init_error = this->_loadouts.initialize();
    if (init_error != FT_ERR_SUCCESS)
    {
        (void)this->_item_definitions.destroy();
        (void)this->_recipes.destroy();
        this->_initialized_state = ft_data_catalog::_state_destroyed;
        return (init_error);
    }
    this->_item_definitions.clear();
    this->_recipes.clear();
    this->_loadouts.clear();
    this->_initialized_state = ft_data_catalog::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::initialize(const ft_data_catalog &other) noexcept
{
    bool other_locked;
    int lock_error;
    if (other._initialized_state != ft_data_catalog::_state_initialized)
    {
        other.abort_lifecycle_error("ft_data_catalog::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_data_catalog::_state_initialized)
    {
        this->abort_lifecycle_error("ft_data_catalog::initialize(copy)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_item_definitions.initialize() != FT_ERR_SUCCESS
        || this->_recipes.initialize() != FT_ERR_SUCCESS
        || this->_loadouts.initialize() != FT_ERR_SUCCESS)
    {
        (void)this->_item_definitions.destroy();
        (void)this->_recipes.destroy();
        (void)this->_loadouts.destroy();
        this->_initialized_state = ft_data_catalog::_state_destroyed;
        return (FT_ERR_INVALID_STATE);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->_item_definitions.destroy();
        (void)this->_recipes.destroy();
        (void)this->_loadouts.destroy();
        this->_initialized_state = ft_data_catalog::_state_destroyed;
        return (lock_error);
    }
    this->_item_definitions = other._item_definitions;
    this->_recipes = other._recipes;
    this->_loadouts = other._loadouts;
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_data_catalog::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::initialize(ft_data_catalog &&other) noexcept
{
    bool other_locked;
    int lock_error;
    if (other._initialized_state != ft_data_catalog::_state_initialized)
    {
        other.abort_lifecycle_error("ft_data_catalog::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_data_catalog::_state_initialized)
    {
        this->abort_lifecycle_error("ft_data_catalog::initialize(move)",
            "destination object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_item_definitions.initialize() != FT_ERR_SUCCESS
        || this->_recipes.initialize() != FT_ERR_SUCCESS
        || this->_loadouts.initialize() != FT_ERR_SUCCESS)
    {
        (void)this->_item_definitions.destroy();
        (void)this->_recipes.destroy();
        (void)this->_loadouts.destroy();
        this->_initialized_state = ft_data_catalog::_state_destroyed;
        return (FT_ERR_INVALID_STATE);
    }
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->_item_definitions.destroy();
        (void)this->_recipes.destroy();
        (void)this->_loadouts.destroy();
        this->_initialized_state = ft_data_catalog::_state_destroyed;
        return (lock_error);
    }
    this->_item_definitions = other._item_definitions;
    this->_recipes = other._recipes;
    this->_loadouts = other._loadouts;
    other._item_definitions.clear();
    other._recipes.clear();
    other._loadouts.clear();
    other.unlock_internal(other_locked);
    this->_initialized_state = ft_data_catalog::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::destroy() noexcept
{
    if (this->_initialized_state != ft_data_catalog::_state_initialized)
    {
        this->_initialized_state = ft_data_catalog::_state_destroyed;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)this->_item_definitions.destroy();
    (void)this->_recipes.destroy();
    (void)this->_loadouts.destroy();
    this->_initialized_state = ft_data_catalog::_state_destroyed;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

ft_map<int, ft_item_definition> &ft_data_catalog::get_item_definitions() noexcept
{
    this->abort_if_not_initialized("ft_data_catalog::get_item_definitions");
    return (this->_item_definitions);
}

const ft_map<int, ft_item_definition> &ft_data_catalog::get_item_definitions() const noexcept
{
    this->abort_if_not_initialized("ft_data_catalog::get_item_definitions const");
    return (this->_item_definitions);
}

ft_map<int, ft_recipe_blueprint> &ft_data_catalog::get_recipes() noexcept
{
    this->abort_if_not_initialized("ft_data_catalog::get_recipes");
    return (this->_recipes);
}

const ft_map<int, ft_recipe_blueprint> &ft_data_catalog::get_recipes() const noexcept
{
    this->abort_if_not_initialized("ft_data_catalog::get_recipes const");
    return (this->_recipes);
}

ft_map<int, ft_loadout_blueprint> &ft_data_catalog::get_loadouts() noexcept
{
    this->abort_if_not_initialized("ft_data_catalog::get_loadouts");
    return (this->_loadouts);
}

const ft_map<int, ft_loadout_blueprint> &ft_data_catalog::get_loadouts() const noexcept
{
    this->abort_if_not_initialized("ft_data_catalog::get_loadouts const");
    return (this->_loadouts);
}

int ft_data_catalog::register_item_definition(const ft_item_definition &definition) noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;

    this->abort_if_not_initialized("ft_data_catalog::register_item_definition");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    identifier = definition.get_item_id();
    this->_item_definitions.insert(identifier, definition);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::register_recipe(const ft_recipe_blueprint &recipe) noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;

    this->abort_if_not_initialized("ft_data_catalog::register_recipe");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    identifier = recipe.get_recipe_id();
    this->_recipes.insert(identifier, recipe);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::register_loadout(const ft_loadout_blueprint &loadout) noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;

    this->abort_if_not_initialized("ft_data_catalog::register_loadout");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    identifier = loadout.get_loadout_id();
    this->_loadouts.insert(identifier, loadout);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::fetch_item_definition(int item_id, ft_item_definition &definition) const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_data_catalog *self;
    const Pair<int, ft_item_definition> *entry;

    this->abort_if_not_initialized("ft_data_catalog::fetch_item_definition");
    self = this;
    lock_acquired = false;
    lock_error = self->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        self->set_error(lock_error);
        return (lock_error);
    }
    entry = self->_item_definitions.find(item_id);
    if (entry == self->_item_definitions.end())
    {
        self->unlock_internal(lock_acquired);
        self->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    if (definition.initialize(entry->value) != FT_ERR_SUCCESS)
    {
        self->unlock_internal(lock_acquired);
        self->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    self->set_error(FT_ERR_SUCCESS);
    self->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::fetch_recipe(int recipe_id, ft_recipe_blueprint &recipe) const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_data_catalog *self;
    const Pair<int, ft_recipe_blueprint> *entry;

    this->abort_if_not_initialized("ft_data_catalog::fetch_recipe");
    self = this;
    lock_acquired = false;
    lock_error = self->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        self->set_error(lock_error);
        return (lock_error);
    }
    entry = self->_recipes.find(recipe_id);
    if (entry == self->_recipes.end())
    {
        self->unlock_internal(lock_acquired);
        self->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    if (recipe.initialize(entry->value) != FT_ERR_SUCCESS)
    {
        self->unlock_internal(lock_acquired);
        self->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    self->set_error(FT_ERR_SUCCESS);
    self->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_data_catalog::fetch_loadout(int loadout_id, ft_loadout_blueprint &loadout) const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_data_catalog *self;
    const Pair<int, ft_loadout_blueprint> *entry;

    this->abort_if_not_initialized("ft_data_catalog::fetch_loadout");
    self = this;
    lock_acquired = false;
    lock_error = self->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        self->set_error(lock_error);
        return (lock_error);
    }
    entry = self->_loadouts.find(loadout_id);
    if (entry == self->_loadouts.end())
    {
        self->unlock_internal(lock_acquired);
        self->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    if (loadout.initialize(entry->value) != FT_ERR_SUCCESS)
    {
        self->unlock_internal(lock_acquired);
        self->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    self->set_error(FT_ERR_SUCCESS);
    self->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}
