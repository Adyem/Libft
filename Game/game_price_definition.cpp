#include "ft_price_definition.hpp"
#include "game_economy_helpers.hpp"
#include "../Template/move.hpp"

int ft_price_definition::lock_pair(const ft_price_definition &first, const ft_price_definition &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_price_definition *ordered_first;
    const ft_price_definition *ordered_second;
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
        const ft_price_definition *temporary;

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
        game_economy_sleep_backoff();
    }
}

ft_price_definition::ft_price_definition() noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0), _maximum_value(0), _error_code(ER_SUCCESS)
{
    return ;
}

ft_price_definition::ft_price_definition(int item_id, int rarity, int base_value, int minimum_value, int maximum_value) noexcept
    : _item_id(item_id), _rarity(rarity), _base_value(base_value), _minimum_value(minimum_value), _maximum_value(maximum_value),
    _error_code(ER_SUCCESS)
{
    return ;
}

ft_price_definition::ft_price_definition(const ft_price_definition &other) noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0), _maximum_value(0), _error_code(ER_SUCCESS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    entry_errno = ft_errno;
    if (ft_price_definition::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_base_value = other._base_value;
    this->_minimum_value = other._minimum_value;
    this->_maximum_value = other._maximum_value;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return ;
}

ft_price_definition &ft_price_definition::operator=(const ft_price_definition &other) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    if (ft_price_definition::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_base_value = other._base_value;
    this->_minimum_value = other._minimum_value;
    this->_maximum_value = other._maximum_value;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_price_definition::ft_price_definition(ft_price_definition &&other) noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0), _maximum_value(0), _error_code(ER_SUCCESS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    entry_errno = ft_errno;
    if (ft_price_definition::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_base_value = other._base_value;
    this->_minimum_value = other._minimum_value;
    this->_maximum_value = other._maximum_value;
    this->_error_code = other._error_code;
    other._item_id = 0;
    other._rarity = 0;
    other._base_value = 0;
    other._minimum_value = 0;
    other._maximum_value = 0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return ;
}

ft_price_definition &ft_price_definition::operator=(ft_price_definition &&other) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    if (ft_price_definition::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_base_value = other._base_value;
    this->_minimum_value = other._minimum_value;
    this->_maximum_value = other._maximum_value;
    this->_error_code = other._error_code;
    other._item_id = 0;
    other._rarity = 0;
    other._base_value = 0;
    other._minimum_value = 0;
    other._maximum_value = 0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_price_definition::get_item_id() const noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    identifier = this->_item_id;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (identifier);
}

void ft_price_definition::set_item_id(int item_id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_item_id = item_id;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

int ft_price_definition::get_rarity() const noexcept
{
    int entry_errno;
    int rarity_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    rarity_value = this->_rarity;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (rarity_value);
}

void ft_price_definition::set_rarity(int rarity) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_rarity = rarity;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

int ft_price_definition::get_base_value() const noexcept
{
    int entry_errno;
    int base_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    base_value = this->_base_value;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (base_value);
}

void ft_price_definition::set_base_value(int base_value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_base_value = base_value;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

int ft_price_definition::get_minimum_value() const noexcept
{
    int entry_errno;
    int minimum_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    minimum_value = this->_minimum_value;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (minimum_value);
}

void ft_price_definition::set_minimum_value(int minimum_value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_minimum_value = minimum_value;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

int ft_price_definition::get_maximum_value() const noexcept
{
    int entry_errno;
    int maximum_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    maximum_value = this->_maximum_value;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (maximum_value);
}

void ft_price_definition::set_maximum_value(int maximum_value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_maximum_value = maximum_value;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

int ft_price_definition::get_error() const noexcept
{
    int entry_errno;
    int error_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_value = this->_error_code;
    const_cast<ft_price_definition *>(this)->set_error(error_value);
    game_economy_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *ft_price_definition::get_error_str() const noexcept
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

void ft_price_definition::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}
