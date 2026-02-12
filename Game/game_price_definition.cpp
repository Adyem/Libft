#include "ft_price_definition.hpp"
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

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESS;
        return (FT_ERR_SUCCESS);
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

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
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
            ft_errno = FT_ERR_SUCCESS;
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_price_definition::ft_price_definition() noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0), _maximum_value(0),
    _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_price_definition::ft_price_definition(int item_id, int rarity, int base_value, int minimum_value, int maximum_value) noexcept
    : _item_id(item_id), _rarity(rarity), _base_value(base_value), _minimum_value(minimum_value), _maximum_value(maximum_value),
    _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_price_definition::ft_price_definition(const ft_price_definition &other) noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0), _maximum_value(0),
    _error_code(FT_ERR_SUCCESS)
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (ft_price_definition::lock_pair(*this, other, self_guard, other_guard)
            != FT_ERR_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        return ;
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_base_value = other._base_value;
    this->_minimum_value = other._minimum_value;
    this->_maximum_value = other._maximum_value;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    return ;
}

ft_price_definition &ft_price_definition::operator=(const ft_price_definition &other) noexcept
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    if (ft_price_definition::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        return (*this);
    }
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_base_value = other._base_value;
    this->_minimum_value = other._minimum_value;
    this->_maximum_value = other._maximum_value;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    return (*this);
}

ft_price_definition::ft_price_definition(ft_price_definition &&other) noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0), _maximum_value(0), _error_code(FT_ERR_SUCCESS)
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (ft_price_definition::lock_pair(*this, other, self_guard, other_guard)
            != FT_ERR_SUCCESS)
    {
        this->set_error(self_guard.get_error());
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
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    return ;
}

ft_price_definition &ft_price_definition::operator=(ft_price_definition &&other) noexcept
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    if (ft_price_definition::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESS)
    {
        this->set_error(self_guard.get_error());
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
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    return (*this);
}

int ft_price_definition::get_item_id() const noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    identifier = this->_item_id;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    return (identifier);
}

void ft_price_definition::set_item_id(int item_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_item_id = item_id;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_rarity() const noexcept
{
    int rarity_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    rarity_value = this->_rarity;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    return (rarity_value);
}

void ft_price_definition::set_rarity(int rarity) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_rarity = rarity;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_base_value() const noexcept
{
    int base_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    base_value = this->_base_value;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    return (base_value);
}

void ft_price_definition::set_base_value(int base_value) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_base_value = base_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_minimum_value() const noexcept
{
    int minimum_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    minimum_value = this->_minimum_value;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    return (minimum_value);
}

void ft_price_definition::set_minimum_value(int minimum_value) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_minimum_value = minimum_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_maximum_value() const noexcept
{
    int maximum_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    maximum_value = this->_maximum_value;
    const_cast<ft_price_definition *>(this)->set_error(this->_error_code);
    return (maximum_value);
}

void ft_price_definition::set_maximum_value(int maximum_value) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_maximum_value = maximum_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_error() const noexcept
{
    int error_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_price_definition *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_value = this->_error_code;
    const_cast<ft_price_definition *>(this)->set_error(error_value);
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
