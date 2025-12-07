#include "game_inventory.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_inventory_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_inventory_set_errno(ft_unique_lock<pt_mutex> &guard,
    int errno_value)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = errno_value;
    return ;
}

int ft_inventory::lock_pair(const ft_inventory &first, const ft_inventory &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_inventory *ordered_first;
    const ft_inventory *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_inventory *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_inventory_sleep_backoff();
    }
}

ft_inventory::ft_inventory(size_t capacity, int weight_limit) noexcept
    : _items(), _capacity(capacity), _used_slots(0),
      _weight_limit(weight_limit), _current_weight(0),
      _next_slot(0), _error(ER_SUCCESS), _mutex()
{
    if (this->handle_items_error() == true)
        return ;
    this->set_error(ER_SUCCESS);
    return ;
}

ft_inventory::ft_inventory(const ft_inventory &other) noexcept
    : _items(), _capacity(0), _used_slots(0),
      _weight_limit(0), _current_weight(0),
      _next_slot(0), _error(ER_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->_items = ft_map<int, ft_sharedptr<ft_item> >();
        this->_capacity = 0;
        this->_used_slots = 0;
        this->_weight_limit = 0;
        this->_current_weight = 0;
        this->_next_slot = 0;
        this->set_error(other_guard.get_error());
        game_inventory_set_errno(other_guard, other_guard.get_error());
        return ;
    }
    this->_items = other._items;
    this->_capacity = other._capacity;
    this->_used_slots = other._used_slots;
    this->_weight_limit = other._weight_limit;
    this->_current_weight = other._current_weight;
    this->_next_slot = other._next_slot;
    this->_error = other._error;
    this->set_error(other._error);
    game_inventory_set_errno(other_guard, ER_SUCCESS);
    return ;
}

ft_inventory &ft_inventory::operator=(const ft_inventory &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_inventory::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_items = other._items;
    this->_capacity = other._capacity;
    this->_used_slots = other._used_slots;
    this->_weight_limit = other._weight_limit;
    this->_current_weight = other._current_weight;
    this->_next_slot = other._next_slot;
    this->_error = other._error;
    this->set_error(other._error);
    game_inventory_set_errno(this_guard, ER_SUCCESS);
    game_inventory_set_errno(other_guard, ER_SUCCESS);
    return (*this);
}

ft_inventory::ft_inventory(ft_inventory &&other) noexcept
    : _items(), _capacity(0), _used_slots(0),
      _weight_limit(0), _current_weight(0),
      _next_slot(0), _error(ER_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->_items = ft_map<int, ft_sharedptr<ft_item> >();
        this->_capacity = 0;
        this->_used_slots = 0;
        this->_weight_limit = 0;
        this->_current_weight = 0;
        this->_next_slot = 0;
        this->set_error(other_guard.get_error());
        game_inventory_set_errno(other_guard, other_guard.get_error());
        return ;
    }
    this->_items = ft_move(other._items);
    this->_capacity = other._capacity;
    this->_used_slots = other._used_slots;
    this->_weight_limit = other._weight_limit;
    this->_current_weight = other._current_weight;
    this->_next_slot = other._next_slot;
    this->_error = other._error;
    other._capacity = 0;
    other._used_slots = 0;
    other._weight_limit = 0;
    other._current_weight = 0;
    other._next_slot = 0;
    other._error = ER_SUCCESS;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    game_inventory_set_errno(other_guard, ER_SUCCESS);
    return ;
}

ft_inventory &ft_inventory::operator=(ft_inventory &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_inventory::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_items = ft_move(other._items);
    this->_capacity = other._capacity;
    this->_used_slots = other._used_slots;
    this->_weight_limit = other._weight_limit;
    this->_current_weight = other._current_weight;
    this->_next_slot = other._next_slot;
    this->_error = other._error;
    other._capacity = 0;
    other._used_slots = 0;
    other._weight_limit = 0;
    other._current_weight = 0;
    other._next_slot = 0;
    other._error = ER_SUCCESS;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    game_inventory_set_errno(this_guard, ER_SUCCESS);
    game_inventory_set_errno(other_guard, ER_SUCCESS);
    return (*this);
}

ft_map<int, ft_sharedptr<ft_item> > &ft_inventory::get_items() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (this->_items);
    }
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (this->_items);
}

const ft_map<int, ft_sharedptr<ft_item> > &ft_inventory::get_items() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (this->_items);
    }
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (this->_items);
}

size_t ft_inventory::get_capacity() const noexcept
{
    size_t capacity_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (0);
    }
    capacity_value = this->_capacity;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (capacity_value);
}

void ft_inventory::resize(size_t capacity) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return ;
    }
    this->_capacity = capacity;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return ;
}

size_t ft_inventory::get_used() const noexcept
{
    size_t used_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (0);
    }
    used_value = this->_used_slots;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (used_value);
}

void ft_inventory::set_used_slots(size_t used) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return ;
    }
    this->_used_slots = used;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return ;
}

bool ft_inventory::is_full() const noexcept
{
    bool result;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (false);
    }
    result = false;
#if USE_INVENTORY_SLOTS
    if (this->_capacity != 0 && this->_used_slots >= this->_capacity)
        result = true;
#endif
#if USE_INVENTORY_WEIGHT
    if (result == false)
    {
        if (this->_weight_limit != 0 && this->_current_weight >= this->_weight_limit)
            result = true;
    }
#endif
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (result);
}

int ft_inventory::get_weight_limit() const noexcept
{
    int limit_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (0);
    }
    limit_value = this->_weight_limit;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (limit_value);
}

void ft_inventory::set_weight_limit(int limit) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return ;
    }
    this->_weight_limit = limit;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return ;
}

int ft_inventory::get_current_weight() const noexcept
{
    int weight_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (0);
    }
    weight_value = this->_current_weight;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (weight_value);
}

void ft_inventory::set_current_weight(int weight) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return ;
    }
    this->_current_weight = weight;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return ;
}

int ft_inventory::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (error_code);
}

const char *ft_inventory::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (ft_strerror(error_code));
}

void ft_inventory::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

bool ft_inventory::handle_items_error() noexcept
{
    int items_error;

    items_error = this->_items.get_error();
    if (items_error == ER_SUCCESS)
        return (false);
    this->set_error(items_error);
    return (true);
}

bool ft_inventory::check_item_errors(const ft_sharedptr<ft_item> &item) const noexcept
{
    if (!item)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    if (item.get_error() != ER_SUCCESS)
    {
        this->set_error(item.get_error());
        return (true);
    }
    if (item->get_error() != ER_SUCCESS)
    {
        this->set_error(item->get_error());
        return (true);
    }
    return (false);
}

int ft_inventory::add_item(const ft_sharedptr<ft_item> &item) noexcept
{
    int remaining;
    int item_id;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (guard.get_error());
    }
    this->set_error(ER_SUCCESS);
    if (this->check_item_errors(item) == true)
    {
        game_inventory_set_errno(guard, this->_error);
        return (this->_error);
    }
    remaining = item->get_stack_size();
#if USE_INVENTORY_WEIGHT
    int projected_weight;

    projected_weight = item->get_stack_size() * item->get_width() * item->get_height();
    if (this->_weight_limit != 0 && this->_current_weight + projected_weight > this->_weight_limit)
    {
        this->set_error(FT_ERR_FULL);
        game_inventory_set_errno(guard, FT_ERR_FULL);
        return (FT_ERR_FULL);
    }
#endif
    item_id = item->get_item_id();

    Pair<int, ft_sharedptr<ft_item> > *item_ptr = this->_items.end() - this->_items.size();
    Pair<int, ft_sharedptr<ft_item> > *item_end = this->_items.end();
    while (item_ptr != item_end && remaining > 0)
    {
        if (item_ptr->value)
        {
            if (item_ptr->value->get_item_id() == item_id)
            {
                int free_space = item_ptr->value->get_max_stack() - item_ptr->value->get_stack_size();
                if (free_space > 0)
                {
                    int to_add;

                    if (remaining < free_space)
                        to_add = remaining;
                    else
                        to_add = free_space;
                    item_ptr->value->add_to_stack(to_add);
                    this->_current_weight += to_add;
                    remaining -= to_add;
                }
            }
        }
        ++item_ptr;
    }

    while (remaining > 0)
    {
#if USE_INVENTORY_SLOTS
        int slot_usage;

        slot_usage = item->get_width() * item->get_height();
        if (this->_capacity != 0 && this->_used_slots + slot_usage > this->_capacity)
        {
            this->set_error(FT_ERR_FULL);
            game_inventory_set_errno(guard, FT_ERR_FULL);
            return (FT_ERR_FULL);
        }
#endif
        ft_sharedptr<ft_item> new_item(new ft_item(*item));
        int new_item_error;

        new_item_error = new_item.get_error();
        if (!new_item || new_item_error != ER_SUCCESS)
        {
            if (new_item_error == ER_SUCCESS)
                this->set_error(FT_ERR_GAME_GENERAL_ERROR);
            else
                this->set_error(new_item_error);
            game_inventory_set_errno(guard, this->_error);
            return (this->_error);
        }
        int to_add;

        if (remaining < new_item->get_max_stack())
            to_add = remaining;
        else
            to_add = new_item->get_max_stack();
        new_item->set_stack_size(to_add);
        this->_items.insert(this->_next_slot, new_item);
        if (this->handle_items_error() == true)
        {
            game_inventory_set_errno(guard, this->_error);
            return (this->_error);
        }
        ++this->_next_slot;
        this->_current_weight += to_add;
#if USE_INVENTORY_SLOTS
        this->_used_slots += slot_usage;
#endif
        remaining -= to_add;
    }
    this->set_error(ER_SUCCESS);
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (ER_SUCCESS);
}

void ft_inventory::remove_item(int slot) noexcept
{
    Pair<int, ft_sharedptr<ft_item> > *entry;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return ;
    }
    entry = this->_items.find(slot);
    if (entry != this->_items.end() && entry->value)
    {
        this->_current_weight -= entry->value->get_stack_size();
#if USE_INVENTORY_SLOTS
        this->_used_slots -= entry->value->get_width() * entry->value->get_height();
#endif
    }
    this->_items.remove(slot);
    if (this->handle_items_error() == true)
    {
        game_inventory_set_errno(guard, this->_error);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_inventory_set_errno(guard, ER_SUCCESS);
    return ;
}

int ft_inventory::count_item(int item_id) const noexcept
{
    int total;
    const Pair<int, ft_sharedptr<ft_item> > *item_ptr;
    const Pair<int, ft_sharedptr<ft_item> > *item_end;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (0);
    }
    total = 0;
    item_ptr = this->_items.end() - this->_items.size();
    item_end = this->_items.end();
    while (item_ptr != item_end)
    {
        if (item_ptr->value)
        {
            if (item_ptr->value->get_item_id() == item_id)
                total += item_ptr->value->get_stack_size();
        }
        ++item_ptr;
    }
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (total);
}

bool ft_inventory::has_item(int item_id) const noexcept
{
    return (this->count_item(item_id) > 0);
}

int ft_inventory::count_rarity(int rarity) const noexcept
{
    int total;
    const Pair<int, ft_sharedptr<ft_item> > *item_ptr;
    const Pair<int, ft_sharedptr<ft_item> > *item_end;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_inventory *>(this)->set_error(guard.get_error());
        game_inventory_set_errno(guard, guard.get_error());
        return (0);
    }
    total = 0;
    item_ptr = this->_items.end() - this->_items.size();
    item_end = this->_items.end();
    while (item_ptr != item_end)
    {
        if (item_ptr->value)
        {
            if (item_ptr->value->get_rarity() == rarity)
                total += item_ptr->value->get_stack_size();
        }
        ++item_ptr;
    }
    game_inventory_set_errno(guard, ER_SUCCESS);
    return (total);
}

bool ft_inventory::has_rarity(int rarity) const noexcept
{
    return (this->count_rarity(rarity) > 0);
}

