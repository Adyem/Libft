#include "game_resistance.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_resistance_sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

static void game_resistance_restore_errno(ft_unique_lock<pt_mutex> &guard) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_resistance::ft_resistance() noexcept
    : _percent_value(0), _flat_value(0), _error_code(FT_ERR_SUCCESS), _mutex()
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_resistance::~ft_resistance() noexcept
{
    return ;
}

int ft_resistance::lock_pair(const ft_resistance &first,
        const ft_resistance &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_resistance *ordered_first;
    const ft_resistance *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.last_operation_error() != FT_ERR_SUCCESS)
        {
            return (single_guard.last_operation_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_resistance *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.last_operation_error() != FT_ERR_SUCCESS)
        {
            return (lower_guard.last_operation_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.last_operation_error() == FT_ERR_SUCCESS)
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
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.last_operation_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.last_operation_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_resistance_sleep_backoff();
    }
}

ft_resistance::ft_resistance(const ft_resistance &other) noexcept
    : _percent_value(0), _flat_value(0), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.last_operation_error());
        game_resistance_restore_errno(other_guard);
        return ;
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_resistance_restore_errno(other_guard);
    return ;
}

ft_resistance &ft_resistance::operator=(const ft_resistance &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_resistance::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_resistance_restore_errno(this_guard);
    game_resistance_restore_errno(other_guard);
    return (*this);
}

ft_resistance::ft_resistance(ft_resistance &&other) noexcept
    : _percent_value(0), _flat_value(0), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.last_operation_error());
        game_resistance_restore_errno(other_guard);
        return ;
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_error_code = other._error_code;
    other._percent_value = 0;
    other._flat_value = 0;
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    game_resistance_restore_errno(other_guard);
    return ;
}

ft_resistance &ft_resistance::operator=(ft_resistance &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_resistance::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_error_code = other._error_code;
    other._percent_value = 0;
    other._flat_value = 0;
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    game_resistance_restore_errno(this_guard);
    game_resistance_restore_errno(other_guard);
    return (*this);
}

int ft_resistance::set_percent(int percent_value) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        game_resistance_restore_errno(guard);
        return (guard.last_operation_error());
    }
    this->_percent_value = percent_value;
    this->set_error(FT_ERR_SUCCESS);
    game_resistance_restore_errno(guard);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::set_flat(int flat_value) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        game_resistance_restore_errno(guard);
        return (guard.last_operation_error());
    }
    this->_flat_value = flat_value;
    this->set_error(FT_ERR_SUCCESS);
    game_resistance_restore_errno(guard);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::set_values(int percent_value, int flat_value) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        game_resistance_restore_errno(guard);
        return (guard.last_operation_error());
    }
    this->_percent_value = percent_value;
    this->_flat_value = flat_value;
    this->set_error(FT_ERR_SUCCESS);
    game_resistance_restore_errno(guard);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::reset() noexcept
{
    return (this->set_values(0, 0));
}

int ft_resistance::get_percent() const noexcept
{
    int percent_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(guard.last_operation_error());
        game_resistance_restore_errno(guard);
        return (0);
    }
    percent_value = this->_percent_value;
    const_cast<ft_resistance *>(this)->set_error(FT_ERR_SUCCESS);
    game_resistance_restore_errno(guard);
    return (percent_value);
}

int ft_resistance::get_flat() const noexcept
{
    int flat_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(guard.last_operation_error());
        game_resistance_restore_errno(guard);
        return (0);
    }
    flat_value = this->_flat_value;
    const_cast<ft_resistance *>(this)->set_error(FT_ERR_SUCCESS);
    game_resistance_restore_errno(guard);
    return (flat_value);
}

int ft_resistance::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(guard.last_operation_error());
        game_resistance_restore_errno(guard);
        return (guard.last_operation_error());
    }
    error_code = this->_error_code;
    const_cast<ft_resistance *>(this)->set_error(error_code);
    game_resistance_restore_errno(guard);
    return (error_code);
}

const char *ft_resistance::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(guard.last_operation_error());
        game_resistance_restore_errno(guard);
        return (ft_strerror(guard.last_operation_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_resistance *>(this)->set_error(error_code);
    game_resistance_restore_errno(guard);
    return (ft_strerror(error_code));
}

void ft_resistance::set_error(int error) const noexcept
{
    this->_error_code = error;
    return ;
}
