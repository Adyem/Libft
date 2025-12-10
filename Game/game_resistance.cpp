#include "game_resistance.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_resistance_sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

static void game_resistance_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

ft_resistance::ft_resistance() noexcept
    : _percent_value(0), _flat_value(0), _error_code(FT_ER_SUCCESSS), _mutex()
{
    this->set_error(FT_ER_SUCCESSS);
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

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
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

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
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
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_resistance_sleep_backoff();
    }
}

ft_resistance::ft_resistance(const ft_resistance &other) noexcept
    : _percent_value(0), _flat_value(0), _error_code(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_resistance_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_resistance_restore_errno(other_guard, entry_errno);
    return ;
}

ft_resistance &ft_resistance::operator=(const ft_resistance &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_resistance::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_resistance_restore_errno(this_guard, entry_errno);
    game_resistance_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_resistance::ft_resistance(ft_resistance &&other) noexcept
    : _percent_value(0), _flat_value(0), _error_code(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_resistance_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_error_code = other._error_code;
    other._percent_value = 0;
    other._flat_value = 0;
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    game_resistance_restore_errno(other_guard, entry_errno);
    return ;
}

ft_resistance &ft_resistance::operator=(ft_resistance &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_resistance::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_percent_value = other._percent_value;
    this->_flat_value = other._flat_value;
    this->_error_code = other._error_code;
    other._percent_value = 0;
    other._flat_value = 0;
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    game_resistance_restore_errno(this_guard, entry_errno);
    game_resistance_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_resistance::set_percent(int percent_value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_resistance_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->_percent_value = percent_value;
    this->set_error(FT_ER_SUCCESSS);
    game_resistance_restore_errno(guard, entry_errno);
    return (FT_ER_SUCCESSS);
}

int ft_resistance::set_flat(int flat_value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_resistance_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->_flat_value = flat_value;
    this->set_error(FT_ER_SUCCESSS);
    game_resistance_restore_errno(guard, entry_errno);
    return (FT_ER_SUCCESSS);
}

int ft_resistance::set_values(int percent_value, int flat_value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_resistance_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->_percent_value = percent_value;
    this->_flat_value = flat_value;
    this->set_error(FT_ER_SUCCESSS);
    game_resistance_restore_errno(guard, entry_errno);
    return (FT_ER_SUCCESSS);
}

int ft_resistance::reset() noexcept
{
    return (this->set_values(0, 0));
}

int ft_resistance::get_percent() const noexcept
{
    int entry_errno;
    int percent_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_resistance *>(this)->set_error(guard.get_error());
        game_resistance_restore_errno(guard, entry_errno);
        return (0);
    }
    percent_value = this->_percent_value;
    const_cast<ft_resistance *>(this)->set_error(FT_ER_SUCCESSS);
    game_resistance_restore_errno(guard, entry_errno);
    return (percent_value);
}

int ft_resistance::get_flat() const noexcept
{
    int entry_errno;
    int flat_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_resistance *>(this)->set_error(guard.get_error());
        game_resistance_restore_errno(guard, entry_errno);
        return (0);
    }
    flat_value = this->_flat_value;
    const_cast<ft_resistance *>(this)->set_error(FT_ER_SUCCESSS);
    game_resistance_restore_errno(guard, entry_errno);
    return (flat_value);
}

int ft_resistance::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_resistance *>(this)->set_error(guard.get_error());
        game_resistance_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_resistance *>(this)->set_error(error_code);
    game_resistance_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_resistance::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_resistance *>(this)->set_error(guard.get_error());
        game_resistance_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_resistance *>(this)->set_error(error_code);
    game_resistance_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_resistance::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}
