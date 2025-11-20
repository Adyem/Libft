#include "ft_behavior_action.hpp"
#include "game_behavior_helpers.hpp"
#include "../Template/move.hpp"

int ft_behavior_action::lock_pair(const ft_behavior_action &first, const ft_behavior_action &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_behavior_action *ordered_first;
    const ft_behavior_action *ordered_second;
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
        const ft_behavior_action *temporary;

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
        game_behavior_sleep_backoff();
    }
}

ft_behavior_action::ft_behavior_action() noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0), _error_code(ER_SUCCESS)
{
    return ;
}

ft_behavior_action::ft_behavior_action(int action_id, double weight, double cooldown_seconds) noexcept
    : _action_id(action_id), _weight(weight), _cooldown_seconds(cooldown_seconds), _error_code(ER_SUCCESS)
{
    return ;
}

ft_behavior_action::ft_behavior_action(const ft_behavior_action &other) noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0), _error_code(ER_SUCCESS)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_behavior_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_error_code = other._error_code;
    game_behavior_restore_errno(other_guard, entry_errno);
    return ;
}

ft_behavior_action &ft_behavior_action::operator=(const ft_behavior_action &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_behavior_action::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_error_code = other._error_code;
    game_behavior_restore_errno(this_guard, entry_errno);
    game_behavior_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_behavior_action::ft_behavior_action(ft_behavior_action &&other) noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0), _error_code(ER_SUCCESS)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_behavior_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_error_code = other._error_code;
    other._action_id = 0;
    other._weight = 0.0;
    other._cooldown_seconds = 0.0;
    other._error_code = ER_SUCCESS;
    game_behavior_restore_errno(other_guard, entry_errno);
    return ;
}

ft_behavior_action &ft_behavior_action::operator=(ft_behavior_action &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_behavior_action::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_error_code = other._error_code;
    other._action_id = 0;
    other._weight = 0.0;
    other._cooldown_seconds = 0.0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_behavior_restore_errno(this_guard, entry_errno);
    game_behavior_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_behavior_action::get_action_id() const noexcept
{
    int entry_errno;
    int action_id;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (0);
    }
    action_id = this->_action_id;
    const_cast<ft_behavior_action *>(this)->set_error(this->_error_code);
    game_behavior_restore_errno(guard, entry_errno);
    return (action_id);
}

void ft_behavior_action::set_action_id(int action_id) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return ;
    }
    this->_action_id = action_id;
    this->set_error(ER_SUCCESS);
    game_behavior_restore_errno(guard, entry_errno);
    return ;
}

double ft_behavior_action::get_weight() const noexcept
{
    int entry_errno;
    double weight;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (0.0);
    }
    weight = this->_weight;
    const_cast<ft_behavior_action *>(this)->set_error(this->_error_code);
    game_behavior_restore_errno(guard, entry_errno);
    return (weight);
}

void ft_behavior_action::set_weight(double weight) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return ;
    }
    this->_weight = weight;
    this->set_error(ER_SUCCESS);
    game_behavior_restore_errno(guard, entry_errno);
    return ;
}

double ft_behavior_action::get_cooldown_seconds() const noexcept
{
    int entry_errno;
    double cooldown_seconds;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (0.0);
    }
    cooldown_seconds = this->_cooldown_seconds;
    const_cast<ft_behavior_action *>(this)->set_error(this->_error_code);
    game_behavior_restore_errno(guard, entry_errno);
    return (cooldown_seconds);
}

void ft_behavior_action::set_cooldown_seconds(double cooldown_seconds) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return ;
    }
    this->_cooldown_seconds = cooldown_seconds;
    this->set_error(ER_SUCCESS);
    game_behavior_restore_errno(guard, entry_errno);
    return ;
}

int ft_behavior_action::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    game_behavior_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_behavior_action::get_error_str() const noexcept
{
    int error_code;
    int entry_errno;

    entry_errno = ft_errno;
    error_code = this->get_error();
    ft_errno = entry_errno;
    return (ft_strerror(error_code));
}

void ft_behavior_action::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
