#include "game_event.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

#include <climits>
#include <utility>

static void game_event_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static int game_event_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno, int result_errno)
{
    int current_errno;
    int final_errno;
    int unlock_errno;

    current_errno = ft_errno;
    final_errno = result_errno;
    if (final_errno == ER_SUCCESS)
    {
        final_errno = current_errno;
    }
    if (guard.owns_lock())
    {
        guard.unlock();
    }
    unlock_errno = guard.get_error();
    if (unlock_errno != ER_SUCCESS)
    {
        ft_errno = unlock_errno;
        return (unlock_errno);
    }
    if (final_errno != ER_SUCCESS)
    {
        ft_errno = final_errno;
        return (final_errno);
    }
    if (entry_errno != ER_SUCCESS)
    {
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

int ft_event::lock_pair(const ft_event &first, const ft_event &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_event *ordered_first;
    const ft_event *ordered_second;
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
        const ft_event *temporary;

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
        game_event_sleep_backoff();
    }
}

ft_event::ft_event() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _callback(), _error(ER_SUCCESS), _mutex()
{
    if (this->_callback.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_callback.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

ft_event::~ft_event() noexcept
{
    return ;
}

ft_event::ft_event(const ft_event &other) noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _callback(), _error(ER_SUCCESS), _mutex()
{
    int entry_errno;
    int callback_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_event_restore_errno(other_guard, entry_errno, ft_errno);
        return ;
    }
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_callback = other._callback;
    callback_error = this->_callback.get_error();
    if (callback_error != ER_SUCCESS)
    {
        this->set_error(callback_error);
        game_event_restore_errno(other_guard, entry_errno, ft_errno);
        return ;
    }
    this->_error = other._error;
    this->set_error(this->_error);
    game_event_restore_errno(other_guard, entry_errno, ft_errno);
    return ;
}

ft_event &ft_event::operator=(const ft_event &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    int callback_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_event::lock_pair(*this, other, this_guard, other_guard);
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
    this->_callback = other._callback;
    callback_error = this->_callback.get_error();
    if (callback_error != ER_SUCCESS)
    {
        this->set_error(callback_error);
        game_event_restore_errno(this_guard, entry_errno, ft_errno);
        game_event_restore_errno(other_guard, entry_errno, ft_errno);
        return (*this);
    }
    this->_error = other._error;
    this->set_error(other._error);
    game_event_restore_errno(this_guard, entry_errno, ft_errno);
    game_event_restore_errno(other_guard, entry_errno, ft_errno);
    return (*this);
}

ft_event::ft_event(ft_event &&other) noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _callback(), _error(ER_SUCCESS), _mutex()
{
    int entry_errno;
    int callback_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_event_restore_errno(other_guard, entry_errno, ft_errno);
        return ;
    }
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_callback = ft_move(other._callback);
    callback_error = this->_callback.get_error();
    if (callback_error != ER_SUCCESS)
    {
        this->set_error(callback_error);
        other._id = 0;
        other._duration = 0;
        other._modifier1 = 0;
        other._modifier2 = 0;
        other._modifier3 = 0;
        other._modifier4 = 0;
        other.set_error(ER_SUCCESS);
        game_event_restore_errno(other_guard, entry_errno, ft_errno);
        return ;
    }
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
    game_event_restore_errno(other_guard, entry_errno, ft_errno);
    return ;
}

ft_event &ft_event::operator=(ft_event &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    int callback_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_event::lock_pair(*this, other, this_guard, other_guard);
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
    this->_callback = ft_move(other._callback);
    callback_error = this->_callback.get_error();
    if (callback_error != ER_SUCCESS)
    {
        this->set_error(callback_error);
        other._id = 0;
        other._duration = 0;
        other._modifier1 = 0;
        other._modifier2 = 0;
        other._modifier3 = 0;
        other._modifier4 = 0;
        other.set_error(ER_SUCCESS);
        game_event_restore_errno(this_guard, entry_errno, ft_errno);
        game_event_restore_errno(other_guard, entry_errno, ft_errno);
        return (*this);
    }
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
    game_event_restore_errno(this_guard, entry_errno, ft_errno);
    game_event_restore_errno(other_guard, entry_errno, ft_errno);
    return (*this);
}

int ft_event::get_id() const noexcept
{
    int entry_errno;
    int identifier;
    int restore_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        restore_error = guard.get_error();
        const_cast<ft_event *>(this)->set_error(restore_error);
        restore_error = game_event_restore_errno(guard, entry_errno, restore_error);
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    identifier = this->_id;
    const_cast<ft_event *>(this)->set_error(ER_SUCCESS);
    restore_error = game_event_restore_errno(guard, entry_errno, ER_SUCCESS);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    return (identifier);
}

void ft_event::set_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_id = id;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

int ft_event::get_duration() const noexcept
{
    int entry_errno;
    int duration_value;
    int previous_error;
    int restore_error;

    entry_errno = ft_errno;
    previous_error = this->_error;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        restore_error = guard.get_error();
        const_cast<ft_event *>(this)->set_error(restore_error);
        restore_error = game_event_restore_errno(guard, entry_errno, restore_error);
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    duration_value = this->_duration;
    restore_error = game_event_restore_errno(guard, entry_errno, ER_SUCCESS);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    const_cast<ft_event *>(this)->set_error(previous_error);
    return (duration_value);
}

void ft_event::set_duration(int duration) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    if (duration < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_duration = duration;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

int ft_event::add_duration(int duration) noexcept
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        result = guard.get_error();
        this->set_error(result);
        return (game_event_restore_errno(guard, entry_errno, result));
    }
    if (duration < 0)
    {
        result = FT_ERR_INVALID_ARGUMENT;
        this->set_error(result);
        return (game_event_restore_errno(guard, entry_errno, result));
    }
    if (duration > 0)
    {
        if (this->_duration > INT_MAX - duration)
        {
            result = FT_ERR_OUT_OF_RANGE;
            this->set_error(result);
            return (game_event_restore_errno(guard, entry_errno, result));
        }
    }
    this->_duration += duration;
    this->set_error(ER_SUCCESS);
    result = ER_SUCCESS;
    return (game_event_restore_errno(guard, entry_errno, result));
}

void ft_event::sub_duration(int duration) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    if (duration < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    if (duration > this->_duration)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_duration -= duration;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

int ft_event::get_modifier1() const noexcept
{
    int entry_errno;
    int modifier_value;
    int restore_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        restore_error = guard.get_error();
        const_cast<ft_event *>(this)->set_error(restore_error);
        restore_error = game_event_restore_errno(guard, entry_errno, restore_error);
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    modifier_value = this->_modifier1;
    const_cast<ft_event *>(this)->set_error(ER_SUCCESS);
    restore_error = game_event_restore_errno(guard, entry_errno, ER_SUCCESS);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    return (modifier_value);
}

void ft_event::set_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier1 = mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

void ft_event::add_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier1 += mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

void ft_event::sub_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier1 -= mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

int ft_event::get_modifier2() const noexcept
{
    int entry_errno;
    int modifier_value;
    int restore_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        restore_error = guard.get_error();
        const_cast<ft_event *>(this)->set_error(restore_error);
        restore_error = game_event_restore_errno(guard, entry_errno, restore_error);
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    modifier_value = this->_modifier2;
    const_cast<ft_event *>(this)->set_error(ER_SUCCESS);
    restore_error = game_event_restore_errno(guard, entry_errno, ER_SUCCESS);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    return (modifier_value);
}

void ft_event::set_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier2 = mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

void ft_event::add_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier2 += mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

void ft_event::sub_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier2 -= mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

int ft_event::get_modifier3() const noexcept
{
    int entry_errno;
    int modifier_value;
    int restore_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        restore_error = guard.get_error();
        const_cast<ft_event *>(this)->set_error(restore_error);
        restore_error = game_event_restore_errno(guard, entry_errno, restore_error);
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    modifier_value = this->_modifier3;
    const_cast<ft_event *>(this)->set_error(ER_SUCCESS);
    restore_error = game_event_restore_errno(guard, entry_errno, ER_SUCCESS);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    return (modifier_value);
}

void ft_event::set_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier3 = mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

void ft_event::add_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier3 += mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

void ft_event::sub_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier3 -= mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

int ft_event::get_modifier4() const noexcept
{
    int entry_errno;
    int modifier_value;
    int restore_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        restore_error = guard.get_error();
        const_cast<ft_event *>(this)->set_error(restore_error);
        restore_error = game_event_restore_errno(guard, entry_errno, restore_error);
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    modifier_value = this->_modifier4;
    const_cast<ft_event *>(this)->set_error(ER_SUCCESS);
    restore_error = game_event_restore_errno(guard, entry_errno, ER_SUCCESS);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (0);
    }
    return (modifier_value);
}

void ft_event::set_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier4 = mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

void ft_event::add_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier4 += mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

void ft_event::sub_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_modifier4 -= mod;
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

const ft_function<void(ft_world&, ft_event&)> &ft_event::get_callback() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_event *>(this)->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return (this->_callback);
    }
    if (this->_callback.get_error() != ER_SUCCESS)
        const_cast<ft_event *>(this)->set_error(this->_callback.get_error());
    else
        const_cast<ft_event *>(this)->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return (this->_callback);
}

void ft_event::set_callback(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    if (callback.get_error() != ER_SUCCESS)
    {
        this->set_error(callback.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->_callback = ft_move(callback);
    if (this->_callback.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_callback.get_error());
        game_event_restore_errno(guard, entry_errno, ft_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_event_restore_errno(guard, entry_errno, ft_errno);
    return ;
}

int ft_event::get_error() const noexcept
{
    int entry_errno;
    int error_code;
    int restore_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        restore_error = guard.get_error();
        const_cast<ft_event *>(this)->set_error(restore_error);
        restore_error = game_event_restore_errno(guard, entry_errno, restore_error);
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (restore_error);
    }
    error_code = this->_error;
    const_cast<ft_event *>(this)->set_error(error_code);
    restore_error = game_event_restore_errno(guard, entry_errno, error_code);
    const_cast<ft_event *>(this)->set_error(restore_error);
    return (restore_error);
}

const char *ft_event::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;
    int restore_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        restore_error = guard.get_error();
        const_cast<ft_event *>(this)->set_error(restore_error);
        restore_error = game_event_restore_errno(guard, entry_errno, restore_error);
        const_cast<ft_event *>(this)->set_error(restore_error);
        return (ft_strerror(restore_error));
    }
    error_code = this->_error;
    const_cast<ft_event *>(this)->set_error(error_code);
    restore_error = game_event_restore_errno(guard, entry_errno, error_code);
    const_cast<ft_event *>(this)->set_error(restore_error);
    return (ft_strerror(restore_error));
}

void ft_event::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
