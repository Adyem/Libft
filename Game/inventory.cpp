#include "inventory.hpp"

ft_inventory::ft_inventory(size_t capacity) noexcept
    : _items(), _capacity(capacity), _next_slot(0), _error(ER_SUCCESS)
{
    if (this->_items.get_error() != ER_SUCCESS)
        this->set_error(this->_items.get_error());
    return ;
}

ft_map<int, ft_item> &ft_inventory::get_items() noexcept
{
    return (this->_items);
}

const ft_map<int, ft_item> &ft_inventory::get_items() const noexcept
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
    return (this->_items.getSize());
}

bool ft_inventory::is_full() const noexcept
{
    return (this->_items.getSize() >= this->_capacity);
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

int ft_inventory::add_item(const ft_item &item) noexcept
{
    this->_error = ER_SUCCESS;
    int remaining = item.get_current_stack();
    int item_id = item.get_item_id();

    Pair<int, ft_item> *ptr = this->_items.end() - this->_items.getSize();
    Pair<int, ft_item> *end = this->_items.end();
    while (ptr != end && remaining > 0)
    {
        if (ptr->value.get_item_id() == item_id)
        {
            int free_space = ptr->value.get_max_stack() - ptr->value.get_current_stack();
            if (free_space > 0)
            {
                int to_add = remaining < free_space ? remaining : free_space;
                ptr->value.add_to_stack(to_add);
                remaining -= to_add;
            }
        }
        ++ptr;
    }

    while (remaining > 0)
    {
        if (this->_items.getSize() >= this->_capacity)
        {
            this->set_error(CHARACTER_INVENTORY_FULL);
            return (CHARACTER_INVENTORY_FULL);
        }
        ft_item new_item = item;
        int to_add = remaining < new_item.get_max_stack() ? remaining : new_item.get_max_stack();
        new_item.set_current_stack(to_add);
        this->_items.insert(this->_next_slot, new_item);
        if (this->_items.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_items.get_error());
            return (this->_error);
        }
        ++this->_next_slot;
        remaining -= to_add;
    }
    return (ER_SUCCESS);
}

void ft_inventory::remove_item(int slot) noexcept
{
    this->_items.remove(slot);
    return ;
}

int ft_inventory::count_item(int item_id) const noexcept
{
    const Pair<int, ft_item> *ptr = this->_items.end() - this->_items.getSize();
    const Pair<int, ft_item> *end = this->_items.end();
    int total = 0;
    while (ptr != end)
    {
        if (ptr->value.get_item_id() == item_id)
            total += ptr->value.get_current_stack();
        ++ptr;
    }
    return (total);
}

bool ft_inventory::has_item(int item_id) const noexcept
{
    return (this->count_item(item_id) > 0);
}

const char *ft_inventory::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

