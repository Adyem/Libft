#include "game_inventory.hpp"

ft_inventory::ft_inventory(size_t capacity, int weight_limit) noexcept
    : _items(), _capacity(capacity), _used_slots(0),
      _weight_limit(weight_limit), _current_weight(0),
      _next_slot(0), _error(ER_SUCCESS)
{
    if (this->_items.get_error() != ER_SUCCESS)
        this->set_error(this->_items.get_error());
    return ;
}

ft_inventory::ft_inventory(const ft_inventory &other) noexcept
    : _items(other._items), _capacity(other._capacity), _used_slots(other._used_slots),
      _weight_limit(other._weight_limit), _current_weight(other._current_weight),
      _next_slot(other._next_slot), _error(other._error)
{
    if (this->_items.get_error() != ER_SUCCESS)
        this->set_error(this->_items.get_error());
    return ;
}

ft_inventory &ft_inventory::operator=(const ft_inventory &other) noexcept
{
    if (this != &other)
    {
        this->_items = other._items;
        this->_capacity = other._capacity;
        this->_used_slots = other._used_slots;
        this->_weight_limit = other._weight_limit;
        this->_current_weight = other._current_weight;
        this->_next_slot = other._next_slot;
        this->_error = other._error;
        if (this->_items.get_error() != ER_SUCCESS)
            this->set_error(this->_items.get_error());
    }
    return (*this);
}

ft_inventory::ft_inventory(ft_inventory &&other) noexcept
    : _items(ft_move(other._items)), _capacity(other._capacity), _used_slots(other._used_slots),
      _weight_limit(other._weight_limit), _current_weight(other._current_weight),
      _next_slot(other._next_slot), _error(other._error)
{
    if (this->_items.get_error() != ER_SUCCESS)
        this->set_error(this->_items.get_error());
    other._capacity = 0;
    other._used_slots = 0;
    other._weight_limit = 0;
    other._current_weight = 0;
    other._next_slot = 0;
    other._error = ER_SUCCESS;
    other._items.clear();
    return ;
}

ft_inventory &ft_inventory::operator=(ft_inventory &&other) noexcept
{
    if (this != &other)
    {
        this->_items = ft_move(other._items);
        this->_capacity = other._capacity;
        this->_used_slots = other._used_slots;
        this->_weight_limit = other._weight_limit;
        this->_current_weight = other._current_weight;
        this->_next_slot = other._next_slot;
        this->_error = other._error;
        if (this->_items.get_error() != ER_SUCCESS)
            this->set_error(this->_items.get_error());
        other._capacity = 0;
        other._used_slots = 0;
        other._weight_limit = 0;
        other._current_weight = 0;
        other._next_slot = 0;
        other._error = ER_SUCCESS;
        other._items.clear();
    }
    return (*this);
}

ft_map<int, ft_sharedptr<ft_item> > &ft_inventory::get_items() noexcept
{
    return (this->_items);
}

const ft_map<int, ft_sharedptr<ft_item> > &ft_inventory::get_items() const noexcept
{
    return (this->_items);
}

size_t ft_inventory::get_capacity() const noexcept
{
    return (this->_capacity);
}

void ft_inventory::resize(size_t capacity) noexcept
{
    this->_capacity = capacity;
    return ;
}

size_t ft_inventory::get_used() const noexcept
{
    return (this->_used_slots);
}

void ft_inventory::set_used_slots(size_t used) noexcept
{
    this->_used_slots = used;
    return ;
}

bool ft_inventory::is_full() const noexcept
{
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
    return (this->_weight_limit);
}

void ft_inventory::set_weight_limit(int limit) noexcept
{
    this->_weight_limit = limit;
    return ;
}

int ft_inventory::get_current_weight() const noexcept
{
    return (this->_current_weight);
}

void ft_inventory::set_current_weight(int weight) noexcept
{
    this->_current_weight = weight;
    return ;
}

int ft_inventory::get_error() const noexcept
{
    return (this->_error);
}

void ft_inventory::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

int ft_inventory::add_item(const ft_sharedptr<ft_item> &item) noexcept
{
    this->_error = ER_SUCCESS;
    if (!item)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (GAME_GENERAL_ERROR);
    }
    if (item.get_error() != ER_SUCCESS)
    {
        this->set_error(item.get_error());
        return (this->_error);
    }
    if (item->get_error() != ER_SUCCESS)
    {
        this->set_error(item->get_error());
        return (this->_error);
    }
    int remaining = item->get_stack_size();
#if USE_INVENTORY_WEIGHT
    if (this->_weight_limit != 0 && this->_current_weight + remaining > this->_weight_limit)
    {
        this->set_error(CHARACTER_INVENTORY_FULL);
        return (CHARACTER_INVENTORY_FULL);
    }
#endif
    int item_id = item->get_item_id();

    Pair<int, ft_sharedptr<ft_item> > *item_ptr = this->_items.end() - this->_items.size();
    Pair<int, ft_sharedptr<ft_item> > *item_end = this->_items.end();
    while (item_ptr != item_end && remaining > 0)
    {
        if (item_ptr->value)
        {
            if (item_ptr->value.get_error() != ER_SUCCESS)
            {
                this->set_error(item_ptr->value.get_error());
                return (this->_error);
            }
            if (item_ptr->value->get_error() != ER_SUCCESS)
            {
                this->set_error(item_ptr->value->get_error());
                return (this->_error);
            }
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
        int slot_usage = item->get_width() * item->get_height();
        if (this->_capacity != 0 && this->_used_slots + slot_usage > this->_capacity)
        {
            this->set_error(CHARACTER_INVENTORY_FULL);
            return (CHARACTER_INVENTORY_FULL);
        }
#endif
        ft_sharedptr<ft_item> new_item(new ft_item(*item));
        if (new_item.get_error() != ER_SUCCESS)
        {
            this->set_error(new_item.get_error());
            return (this->_error);
        }
        if (new_item->get_error() != ER_SUCCESS)
        {
            this->set_error(new_item->get_error());
            return (this->_error);
        }
        int to_add;
        if (remaining < new_item->get_max_stack())
            to_add = remaining;
        else
            to_add = new_item->get_max_stack();
        new_item->set_stack_size(to_add);
        this->_items.insert(this->_next_slot, new_item);
        if (this->_items.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_items.get_error());
            return (this->_error);
        }
        ++this->_next_slot;
        this->_current_weight += to_add;
#if USE_INVENTORY_SLOTS
        this->_used_slots += slot_usage;
#endif
        remaining -= to_add;
    }
    return (ER_SUCCESS);
}

void ft_inventory::remove_item(int slot) noexcept
{
    Pair<int, ft_sharedptr<ft_item> > *entry = this->_items.find(slot);
    if (entry && entry->value)
    {
        if (entry->value.get_error() != ER_SUCCESS)
        {
            this->set_error(entry->value.get_error());
            return ;
        }
        if (entry->value->get_error() == ER_SUCCESS)
        {
            this->_current_weight -= entry->value->get_stack_size();
#if USE_INVENTORY_SLOTS
            this->_used_slots -= entry->value->get_width() * entry->value->get_height();
#endif
        }
    }
    this->_items.remove(slot);
    return ;
}

int ft_inventory::count_item(int item_id) const noexcept
{
    const Pair<int, ft_sharedptr<ft_item> > *item_ptr = this->_items.end() - this->_items.size();
    const Pair<int, ft_sharedptr<ft_item> > *item_end = this->_items.end();
    int total = 0;
    while (item_ptr != item_end)
    {
        if (item_ptr->value)
        {
            if (item_ptr->value.get_error() != ER_SUCCESS)
            {
                this->set_error(item_ptr->value.get_error());
                return (0);
            }
            if (item_ptr->value->get_error() != ER_SUCCESS)
            {
                this->set_error(item_ptr->value->get_error());
                return (0);
            }
            if (item_ptr->value->get_item_id() == item_id)
                total += item_ptr->value->get_stack_size();
        }
        ++item_ptr;
    }
    return (total);
}

bool ft_inventory::has_item(int item_id) const noexcept
{
    return (this->count_item(item_id) > 0);
}

int ft_inventory::count_rarity(int rarity) const noexcept
{
    const Pair<int, ft_sharedptr<ft_item> > *item_ptr = this->_items.end() - this->_items.size();
    const Pair<int, ft_sharedptr<ft_item> > *item_end = this->_items.end();
    int total = 0;
    while (item_ptr != item_end)
    {
        if (item_ptr->value)
        {
            if (item_ptr->value.get_error() != ER_SUCCESS)
            {
                this->set_error(item_ptr->value.get_error());
                return (0);
            }
            if (item_ptr->value->get_error() != ER_SUCCESS)
            {
                this->set_error(item_ptr->value->get_error());
                return (0);
            }
            if (item_ptr->value->get_rarity() == rarity)
                total += item_ptr->value->get_stack_size();
        }
        ++item_ptr;
    }
    return (total);
}

bool ft_inventory::has_rarity(int rarity) const noexcept
{
    return (this->count_rarity(rarity) > 0);
}

const char *ft_inventory::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

