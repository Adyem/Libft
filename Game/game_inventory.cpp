#include "game_inventory.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

ft_inventory::ft_inventory() noexcept
    : _items(), _capacity(0), _used_slots(0), _weight_limit(0),
      _current_weight(0), _next_slot(0), _mutex(ft_nullptr),
      _initialized_state(ft_inventory::_state_uninitialized)
{
    return ;
}

ft_inventory::~ft_inventory() noexcept
{
    if (this->_initialized_state == ft_inventory::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_inventory::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_inventory::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_inventory lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_inventory::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_inventory::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_inventory::initialize() noexcept
{
    if (this->_initialized_state == ft_inventory::_state_initialized)
    {
        this->abort_lifecycle_error("ft_inventory::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_items.clear();
    this->_used_slots = 0;
    this->_current_weight = 0;
    this->_next_slot = 0;
    this->_initialized_state = ft_inventory::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_inventory::initialize(size_t capacity, int weight_limit) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_capacity = capacity;
    this->_weight_limit = weight_limit;
    return (FT_ERR_SUCCESS);
}

int ft_inventory::initialize(const ft_inventory &other) noexcept
{
    int initialize_error;
    size_t count;
    size_t index;
    const Pair<int, ft_sharedptr<ft_item> > *entry;
    const Pair<int, ft_sharedptr<ft_item> > *entry_end;

    if (other._initialized_state != ft_inventory::_state_initialized)
    {
        other.abort_lifecycle_error("ft_inventory::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_capacity = other._capacity;
    this->_used_slots = other._used_slots;
    this->_weight_limit = other._weight_limit;
    this->_current_weight = other._current_weight;
    this->_next_slot = other._next_slot;
    count = other._items.size();
    entry_end = other._items.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_items.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    return (FT_ERR_SUCCESS);
}

int ft_inventory::initialize(ft_inventory &&other) noexcept
{
    return (this->initialize(static_cast<const ft_inventory &>(other)));
}

int ft_inventory::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_inventory::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_items.clear();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_inventory::_state_destroyed;
    return (disable_error);
}

int ft_inventory::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_inventory::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_inventory::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_inventory::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_inventory::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_inventory::lock_internal(bool *lock_acquired) const noexcept
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

int ft_inventory::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_inventory::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_inventory::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_inventory::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_inventory::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int, ft_sharedptr<ft_item> > &ft_inventory::get_items() noexcept
{
    this->abort_if_not_initialized("ft_inventory::get_items");
    return (this->_items);
}

const ft_map<int, ft_sharedptr<ft_item> > &ft_inventory::get_items() const noexcept
{
    this->abort_if_not_initialized("ft_inventory::get_items const");
    return (this->_items);
}

size_t ft_inventory::get_capacity() const noexcept
{
    this->abort_if_not_initialized("ft_inventory::get_capacity");
    return (this->_capacity);
}

void ft_inventory::resize(size_t capacity) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_inventory::resize");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_capacity = capacity;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

size_t ft_inventory::get_used() const noexcept
{
    this->abort_if_not_initialized("ft_inventory::get_used");
    return (this->_used_slots);
}

void ft_inventory::set_used_slots(size_t used) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_inventory::set_used_slots");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_used_slots = used;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

bool ft_inventory::is_full() const noexcept
{
    this->abort_if_not_initialized("ft_inventory::is_full");
#if USE_INVENTORY_SLOTS
    if (this->_capacity != 0 && this->_used_slots >= this->_capacity)
        return (true);
#endif
#if USE_INVENTORY_WEIGHT
    if (this->_weight_limit != 0 && this->_current_weight >= this->_weight_limit)
        return (true);
#endif
    return (false);
}

int ft_inventory::get_weight_limit() const noexcept
{
    this->abort_if_not_initialized("ft_inventory::get_weight_limit");
    return (this->_weight_limit);
}

void ft_inventory::set_weight_limit(int limit) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_inventory::set_weight_limit");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_weight_limit = limit;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_inventory::get_current_weight() const noexcept
{
    this->abort_if_not_initialized("ft_inventory::get_current_weight");
    return (this->_current_weight);
}

void ft_inventory::set_current_weight(int weight) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_inventory::set_current_weight");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_current_weight = weight;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

bool ft_inventory::check_item_valid(const ft_sharedptr<ft_item> &item) const noexcept
{
    if (!item)
        return (false);
    return (true);
}

int ft_inventory::add_item(const ft_sharedptr<ft_item> &item) noexcept
{
    int remaining;
    int item_id;
    Pair<int, ft_sharedptr<ft_item> > *item_pointer;
    Pair<int, ft_sharedptr<ft_item> > *item_end;

    this->abort_if_not_initialized("ft_inventory::add_item");
    if (this->check_item_valid(item) == false)
        return (FT_ERR_INVALID_ARGUMENT);
    remaining = item->get_stack_size();
#if USE_INVENTORY_WEIGHT
    int projected_weight;

    projected_weight = item->get_stack_size() * item->get_width() * item->get_height();
    if (this->_weight_limit != 0 && this->_current_weight + projected_weight > this->_weight_limit)
        return (FT_ERR_FULL);
#endif
    item_id = item->get_item_id();
    item_pointer = this->_items.end() - this->_items.size();
    item_end = this->_items.end();
    while (item_pointer != item_end && remaining > 0)
    {
        if (item_pointer->value)
        {
            if (item_pointer->value->get_item_id() == item_id)
            {
                int free_space;
                int amount_to_add;

                free_space = item_pointer->value->get_max_stack() - item_pointer->value->get_stack_size();
                if (free_space > 0)
                {
                    if (remaining < free_space)
                        amount_to_add = remaining;
                    else
                        amount_to_add = free_space;
                    item_pointer->value->add_to_stack(amount_to_add);
                    this->_current_weight += amount_to_add;
                    remaining -= amount_to_add;
                }
            }
        }
        item_pointer++;
    }
    while (remaining > 0)
    {
#if USE_INVENTORY_SLOTS
        int slot_usage;

        slot_usage = item->get_width() * item->get_height();
        if (this->_capacity != 0 && this->_used_slots + slot_usage > this->_capacity)
            return (FT_ERR_FULL);
#endif
        ft_sharedptr<ft_item> new_item(new ft_item());
        int amount_to_add;

        if (!new_item)
            return (FT_ERR_NO_MEMORY);
        if (new_item->initialize(*item) != FT_ERR_SUCCESS)
            return (new_item->get_error());
        if (remaining < new_item->get_max_stack())
            amount_to_add = remaining;
        else
            amount_to_add = new_item->get_max_stack();
        new_item->set_stack_size(amount_to_add);
        this->_items.insert(this->_next_slot, new_item);
        this->_next_slot++;
        this->_current_weight += amount_to_add;
#if USE_INVENTORY_SLOTS
        this->_used_slots += slot_usage;
#endif
        remaining -= amount_to_add;
    }
    return (FT_ERR_SUCCESS);
}

void ft_inventory::remove_item(int slot) noexcept
{
    Pair<int, ft_sharedptr<ft_item> > *entry;

    this->abort_if_not_initialized("ft_inventory::remove_item");
    entry = this->_items.find(slot);
    if (entry != this->_items.end() && entry->value)
    {
        this->_current_weight -= entry->value->get_stack_size();
#if USE_INVENTORY_SLOTS
        this->_used_slots -= entry->value->get_width() * entry->value->get_height();
#endif
    }
    this->_items.remove(slot);
    return ;
}

int ft_inventory::count_item(int item_id) const noexcept
{
    int total;
    const Pair<int, ft_sharedptr<ft_item> > *item_pointer;
    const Pair<int, ft_sharedptr<ft_item> > *item_end;

    this->abort_if_not_initialized("ft_inventory::count_item");
    total = 0;
    item_pointer = this->_items.end() - this->_items.size();
    item_end = this->_items.end();
    while (item_pointer != item_end)
    {
        if (item_pointer->value)
        {
            if (item_pointer->value->get_item_id() == item_id)
                total += item_pointer->value->get_stack_size();
        }
        item_pointer++;
    }
    return (total);
}

bool ft_inventory::has_item(int item_id) const noexcept
{
    this->abort_if_not_initialized("ft_inventory::has_item");
    return (this->count_item(item_id) > 0);
}

int ft_inventory::count_rarity(int rarity) const noexcept
{
    int total;
    const Pair<int, ft_sharedptr<ft_item> > *item_pointer;
    const Pair<int, ft_sharedptr<ft_item> > *item_end;

    this->abort_if_not_initialized("ft_inventory::count_rarity");
    total = 0;
    item_pointer = this->_items.end() - this->_items.size();
    item_end = this->_items.end();
    while (item_pointer != item_end)
    {
        if (item_pointer->value)
        {
            if (item_pointer->value->get_rarity() == rarity)
                total += item_pointer->value->get_stack_size();
        }
        item_pointer++;
    }
    return (total);
}

bool ft_inventory::has_rarity(int rarity) const noexcept
{
    this->abort_if_not_initialized("ft_inventory::has_rarity");
    return (this->count_rarity(rarity) > 0);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_inventory::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_inventory::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
