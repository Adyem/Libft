#include "../PThread/pthread_internal.hpp"
#include "game_inventory.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local int32_t game_inventory::_last_error = FT_ERR_SUCCESS;

game_inventory::game_inventory() noexcept
    : _items(), _capacity(0), _used_slots(0), _weight_limit(0),
      _current_weight(0), _next_slot(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_inventory::game_inventory(const game_inventory &other) noexcept
    : _items(), _capacity(0), _used_slots(0), _weight_limit(0),
      _current_weight(0), _next_slot(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_inventory::game_inventory(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

game_inventory::game_inventory(game_inventory &&other) noexcept
    : _items(), _capacity(0), _used_slots(0), _weight_limit(0),
      _current_weight(0), _next_slot(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_inventory::game_inventory(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

game_inventory::~game_inventory() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        return ;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_inventory::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_inventory::initialize", "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    const int32_t destroy_error = this->_items.destroy();
    if (destroy_error != FT_ERR_SUCCESS && destroy_error != FT_ERR_INVALID_STATE)
    {
        this->set_error(destroy_error);
        return (destroy_error);
    }
    const int32_t map_initialize_error = this->_items.initialize();
    if (map_initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(map_initialize_error);
        return (map_initialize_error);
    }
    this->_used_slots = 0;
    this->_current_weight = 0;
    this->_next_slot = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_inventory::initialize(ft_size_t capacity, int32_t weight_limit) noexcept
{
    int32_t initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_capacity = capacity;
    this->_weight_limit = weight_limit;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_inventory::initialize(const game_inventory &other) noexcept
{
    int32_t initialize_error;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, ft_sharedptr<game_item> > *entry;
    const Pair<int32_t, ft_sharedptr<game_item> > *entry_end;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_inventory::initialize(copy)", "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        int32_t destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
        {
            this->set_error(destroy_error);
            return (destroy_error);
        }
    }
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
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_inventory::initialize(game_inventory &&other) noexcept
{
    return (this->move(other));
}

int32_t game_inventory::move(game_inventory &other) noexcept
{
    int32_t initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_inventory::move", "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize(static_cast<const game_inventory &>(other));
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    (void)other.destroy();
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t game_inventory::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_items.clear();
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_inventory::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::enable_thread_safety");
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
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_inventory::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_inventory::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_inventory::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_inventory::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t game_inventory::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::lock");
    const int32_t lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void game_inventory::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int32_t, ft_sharedptr<game_item> > &game_inventory::get_items() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::get_items");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_items);
}

const ft_map<int32_t, ft_sharedptr<game_item> > &game_inventory::get_items() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::get_items const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_items);
}

ft_size_t game_inventory::get_capacity() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::get_capacity");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_capacity);
}

void game_inventory::resize(ft_size_t capacity) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::resize");
    const int32_t lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(lock_result);
        return ;
    }
    this->_capacity = capacity;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_size_t game_inventory::get_used() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::get_used");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_used_slots);
}

void game_inventory::set_used_slots(ft_size_t used) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::set_used_slots");
    const int32_t lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(lock_result);
        return ;
    }
    this->_used_slots = used;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_bool game_inventory::is_full() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::is_full");
    ft_bool is_full_result;

#if USE_INVENTORY_SLOTS
    if (this->_capacity != 0 && this->_used_slots >= this->_capacity)
        is_full_result = FT_TRUE;
    else
#endif
#if USE_INVENTORY_WEIGHT
    if (this->_weight_limit != 0 && this->_current_weight >= this->_weight_limit)
        is_full_result = FT_TRUE;
    else
#endif
        is_full_result = FT_FALSE;
    this->set_error(FT_ERR_SUCCESS);
    return (is_full_result);
}

int32_t game_inventory::get_weight_limit() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::get_weight_limit");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_weight_limit);
}

void game_inventory::set_weight_limit(int32_t limit) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::set_weight_limit");
    const int32_t lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(lock_result);
        return ;
    }
    this->_weight_limit = limit;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_inventory::get_current_weight() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::get_current_weight");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_current_weight);
}

void game_inventory::set_current_weight(int32_t weight) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::set_current_weight");
    const int32_t lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(lock_result);
        return ;
    }
    this->_current_weight = weight;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_bool game_inventory::check_item_valid(const ft_sharedptr<game_item> &item) const noexcept
{
    if (!item)
        return (FT_FALSE);
    return (FT_TRUE);
}

int32_t game_inventory::add_item(const ft_sharedptr<game_item> &item) noexcept
{
    int32_t remaining;
    int32_t item_id;
    Pair<int32_t, ft_sharedptr<game_item> > *item_pointer;
    Pair<int32_t, ft_sharedptr<game_item> > *item_end;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::add_item");
    if (this->check_item_valid(item) == FT_FALSE)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    remaining = item->get_stack_size();
#if USE_INVENTORY_WEIGHT
    int32_t projected_weight;

    projected_weight = item->get_stack_size() * item->get_width() * item->get_height();
    if (this->_weight_limit != 0 && this->_current_weight + projected_weight > this->_weight_limit)
    {
        this->set_error(FT_ERR_FULL);
        return (FT_ERR_FULL);
    }
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
                int32_t free_space;
                int32_t amount_to_add;

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
        int32_t slot_usage;

        slot_usage = item->get_width() * item->get_height();
        if (this->_capacity != 0 && this->_used_slots + slot_usage > this->_capacity)
        {
            this->set_error(FT_ERR_FULL);
            return (FT_ERR_FULL);
        }
#endif
        ft_sharedptr<game_item> new_item(new game_item());
        int32_t amount_to_add;

        if (!new_item)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return (FT_ERR_NO_MEMORY);
        }
        const int32_t initialize_error = new_item->initialize(*item);
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->set_error(initialize_error);
            return (initialize_error);
        }
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
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void game_inventory::remove_item(int32_t slot) noexcept
{
    Pair<int32_t, ft_sharedptr<game_item> > *entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::remove_item");
    entry = this->_items.find(slot);
    if (entry != this->_items.end() && entry->value)
    {
        this->_current_weight -= entry->value->get_stack_size();
#if USE_INVENTORY_SLOTS
        this->_used_slots -= entry->value->get_width() * entry->value->get_height();
#endif
    }
    this->_items.remove(slot);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t game_inventory::count_item(int32_t item_id) const noexcept
{
    int32_t total;
    ft_size_t index;
    const Pair<int32_t, ft_sharedptr<game_item> > *item_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::count_item");
    total = 0;
    index = 0;
    while (index < static_cast<ft_size_t>(this->_next_slot))
    {
        item_pointer = this->_items.find(static_cast<int32_t>(index));
        if (item_pointer != this->_items.end() && item_pointer->value)
        {
            if (item_pointer->value->get_item_id() == item_id)
                total += item_pointer->value->get_stack_size();
        }
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (total);
}

ft_bool game_inventory::has_item(int32_t item_id) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::has_item");
    const ft_bool result = this->count_item(item_id) > 0;
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int32_t game_inventory::count_rarity(int32_t rarity) const noexcept
{
    int32_t total;
    ft_size_t index;
    const Pair<int32_t, ft_sharedptr<game_item> > *item_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::count_rarity");
    total = 0;
    index = 0;
    while (index < static_cast<ft_size_t>(this->_next_slot))
    {
        item_pointer = this->_items.find(static_cast<int32_t>(index));
        if (item_pointer != this->_items.end() && item_pointer->value)
        {
            if (item_pointer->value->get_rarity() == rarity)
                total += item_pointer->value->get_stack_size();
        }
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (total);
}

ft_bool game_inventory::has_rarity(int32_t rarity) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_inventory::has_rarity");
    const ft_bool result = this->count_rarity(rarity) > 0;
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int32_t game_inventory::set_error(int32_t error_code) noexcept
{
    game_inventory::_last_error = error_code;
    return (error_code);
}

int32_t game_inventory::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_inventory::get_error");
    return (static_cast<int32_t>(game_inventory::_last_error));
}

const char *game_inventory::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_inventory::get_error_str");
    return (ft_strerror(this->get_error()));
}
