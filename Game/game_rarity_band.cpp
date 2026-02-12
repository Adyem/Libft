#include "ft_rarity_band.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"

thread_local ft_operation_error_stack ft_rarity_band::_operation_errors = {{}, {}, 0};

void ft_rarity_band::record_operation_error_unlocked(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(ft_rarity_band::_operation_errors,
            error_code, operation_id);
    return ;
}

int ft_rarity_band::lock_pair(const ft_rarity_band &first, const ft_rarity_band &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_rarity_band *ordered_first;
    const ft_rarity_band *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_rarity_band::record_operation_error_unlocked(single_guard.get_error());
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_rarity_band::record_operation_error_unlocked(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_rarity_band *temporary;

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
            ft_rarity_band::record_operation_error_unlocked(lower_guard.get_error());
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
            ft_rarity_band::record_operation_error_unlocked(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_rarity_band::record_operation_error_unlocked(upper_guard.get_error());
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_rarity_band::ft_rarity_band() noexcept
    : _rarity(0), _value_multiplier(1.0), _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_rarity_band::ft_rarity_band(int rarity, double value_multiplier) noexcept
    : _rarity(rarity), _value_multiplier(value_multiplier), _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_rarity_band::ft_rarity_band(const ft_rarity_band &other) noexcept
    : _rarity(0), _value_multiplier(1.0), _error_code(FT_ERR_SUCCESS)
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (ft_rarity_band::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        return ;
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    return ;
}

ft_rarity_band &ft_rarity_band::operator=(const ft_rarity_band &other) noexcept
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    if (ft_rarity_band::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        return (*this);
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    return (*this);
}

ft_rarity_band::ft_rarity_band(ft_rarity_band &&other) noexcept
    : _rarity(0), _value_multiplier(1.0), _error_code(FT_ERR_SUCCESS)
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (ft_rarity_band::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        return ;
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->_error_code = other._error_code;
    other._rarity = 0;
    other._value_multiplier = 0.0;
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    return ;
}

ft_rarity_band &ft_rarity_band::operator=(ft_rarity_band &&other) noexcept
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    if (ft_rarity_band::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        return (*this);
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->_error_code = other._error_code;
    other._rarity = 0;
    other._value_multiplier = 0.0;
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    return (*this);
}

int ft_rarity_band::get_rarity() const noexcept
{
    int rarity_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_rarity_band *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    rarity_value = this->_rarity;
    const_cast<ft_rarity_band *>(this)->set_error(this->_error_code);
    return (rarity_value);
}

void ft_rarity_band::set_rarity(int rarity) noexcept
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

double ft_rarity_band::get_value_multiplier() const noexcept
{
    double multiplier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_rarity_band *>(this)->set_error(guard.get_error());
        return (static_cast<double>(guard.get_error()));
    }
    multiplier = this->_value_multiplier;
    const_cast<ft_rarity_band *>(this)->set_error(this->_error_code);
    return (multiplier);
}

void ft_rarity_band::set_value_multiplier(double value_multiplier) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_value_multiplier = value_multiplier;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_rarity_band::get_error() const noexcept
{
    int error_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_rarity_band *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_value = this->_error_code;
    const_cast<ft_rarity_band *>(this)->set_error(error_value);
    return (error_value);
}

const char *ft_rarity_band::get_error_str() const noexcept
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

void ft_rarity_band::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_rarity_band::record_operation_error_unlocked(error_code);
    return ;
}
