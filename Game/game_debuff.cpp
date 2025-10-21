#include "game_debuff.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_debuff_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_debuff_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_debuff::lock_pair(const ft_debuff &first, const ft_debuff &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_debuff *ordered_first;
    const ft_debuff *ordered_second;
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
        const ft_debuff *temporary;

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
        game_debuff_sleep_backoff();
    }
}

ft_debuff::ft_debuff() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _error(ER_SUCCESS), _mutex()
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_debuff::ft_debuff(const ft_debuff &other) noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _error(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_debuff_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    this->set_error(other._error);
    game_debuff_restore_errno(other_guard, entry_errno);
    return ;
}

ft_debuff &ft_debuff::operator=(const ft_debuff &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_debuff::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    this->set_error(other._error);
    game_debuff_restore_errno(this_guard, entry_errno);
    game_debuff_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_debuff::ft_debuff(ft_debuff &&other) noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _error(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_debuff_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    other._id = 0;
    other._duration = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other._error = ER_SUCCESS;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    game_debuff_restore_errno(other_guard, entry_errno);
    return ;
}

ft_debuff &ft_debuff::operator=(ft_debuff &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_debuff::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    other._id = 0;
    other._duration = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other._error = ER_SUCCESS;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    game_debuff_restore_errno(this_guard, entry_errno);
    game_debuff_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_debuff::get_id() const noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_debuff *>(this)->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return (0);
    }
    identifier = this->_id;
    const_cast<ft_debuff *>(this)->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return (identifier);
}

void ft_debuff::set_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_id = id;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

int ft_debuff::get_duration() const noexcept
{
    int entry_errno;
    int duration_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_debuff *>(this)->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return (0);
    }
    duration_value = this->_duration;
    const_cast<ft_debuff *>(this)->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return (duration_value);
}

void ft_debuff::set_duration(int duration) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    if (duration < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_duration = duration;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::add_duration(int duration) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    if (duration < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_duration += duration;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::sub_duration(int duration) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    if (duration < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_duration -= duration;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

int ft_debuff::get_modifier1() const noexcept
{
    int entry_errno;
    int modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_debuff *>(this)->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier = this->_modifier1;
    const_cast<ft_debuff *>(this)->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_debuff::set_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1 = mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::add_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1 += mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::sub_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1 -= mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

int ft_debuff::get_modifier2() const noexcept
{
    int entry_errno;
    int modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_debuff *>(this)->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier = this->_modifier2;
    const_cast<ft_debuff *>(this)->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_debuff::set_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2 = mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::add_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2 += mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::sub_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2 -= mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

int ft_debuff::get_modifier3() const noexcept
{
    int entry_errno;
    int modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_debuff *>(this)->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier = this->_modifier3;
    const_cast<ft_debuff *>(this)->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_debuff::set_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3 = mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::add_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3 += mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::sub_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3 -= mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

int ft_debuff::get_modifier4() const noexcept
{
    int entry_errno;
    int modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_debuff *>(this)->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier = this->_modifier4;
    const_cast<ft_debuff *>(this)->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_debuff::set_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4 = mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::add_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4 += mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

void ft_debuff::sub_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4 -= mod;
    this->set_error(ER_SUCCESS);
    game_debuff_restore_errno(guard, entry_errno);
    return ;
}

int ft_debuff::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_debuff *>(this)->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_debuff *>(this)->set_error(error_code);
    game_debuff_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_debuff::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_debuff *>(this)->set_error(guard.get_error());
        game_debuff_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_debuff *>(this)->set_error(error_code);
    game_debuff_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_debuff::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
