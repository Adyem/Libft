#include "ft_behavior_action.hpp"
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
        const ft_behavior_action *temporary;

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

ft_behavior_action::ft_behavior_action() noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0), _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_behavior_action::ft_behavior_action(int action_id, double weight, double cooldown_seconds) noexcept
    : _action_id(action_id), _weight(weight), _cooldown_seconds(cooldown_seconds), _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_behavior_action::ft_behavior_action(const ft_behavior_action &other) noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0), _error_code(FT_ERR_SUCCESS)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        ft_errno = other_guard.get_error();
        return ;
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_error_code = other._error_code;
    ft_errno = FT_ERR_SUCCESS;
    return ;
}

ft_behavior_action &ft_behavior_action::operator=(const ft_behavior_action &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_behavior_action::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_error_code = other._error_code;
    ft_errno = FT_ERR_SUCCESS;
    return (*this);
}

ft_behavior_action::ft_behavior_action(ft_behavior_action &&other) noexcept
    : _action_id(0), _weight(0.0), _cooldown_seconds(0.0), _error_code(FT_ERR_SUCCESS)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        ft_errno = other_guard.get_error();
        return ;
    }
    this->_action_id = other._action_id;
    this->_weight = other._weight;
    this->_cooldown_seconds = other._cooldown_seconds;
    this->_error_code = other._error_code;
    other._action_id = 0;
    other._weight = 0.0;
    other._cooldown_seconds = 0.0;
    other._error_code = FT_ERR_SUCCESS;
    ft_errno = FT_ERR_SUCCESS;
    return ;
}

ft_behavior_action &ft_behavior_action::operator=(ft_behavior_action &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_behavior_action::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
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
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    ft_errno = FT_ERR_SUCCESS;
    return (*this);
}

int ft_behavior_action::get_action_id() const noexcept
{
    int action_id;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(guard.get_error());
        ft_errno = guard.get_error();
        return (0);
    }
    action_id = this->_action_id;
    const_cast<ft_behavior_action *>(this)->set_error(this->_error_code);
    ft_errno = FT_ERR_SUCCESS;
    return (action_id);
}

void ft_behavior_action::set_action_id(int action_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_errno = guard.get_error();
        return ;
    }
    this->_action_id = action_id;
    this->set_error(FT_ERR_SUCCESS);
    ft_errno = FT_ERR_SUCCESS;
    return ;
}

double ft_behavior_action::get_weight() const noexcept
{
    double weight;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(guard.get_error());
        ft_errno = guard.get_error();
        return (0.0);
    }
    weight = this->_weight;
    const_cast<ft_behavior_action *>(this)->set_error(this->_error_code);
    ft_errno = FT_ERR_SUCCESS;
    return (weight);
}

void ft_behavior_action::set_weight(double weight) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_errno = guard.get_error();
        return ;
    }
    this->_weight = weight;
    this->set_error(FT_ERR_SUCCESS);
    ft_errno = FT_ERR_SUCCESS;
    return ;
}

double ft_behavior_action::get_cooldown_seconds() const noexcept
{
    double cooldown_seconds;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(guard.get_error());
        ft_errno = guard.get_error();
        return (0.0);
    }
    cooldown_seconds = this->_cooldown_seconds;
    const_cast<ft_behavior_action *>(this)->set_error(this->_error_code);
    ft_errno = FT_ERR_SUCCESS;
    return (cooldown_seconds);
}

void ft_behavior_action::set_cooldown_seconds(double cooldown_seconds) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_errno = guard.get_error();
        return ;
    }
    this->_cooldown_seconds = cooldown_seconds;
    this->set_error(FT_ERR_SUCCESS);
    ft_errno = FT_ERR_SUCCESS;
    return ;
}

int ft_behavior_action::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_action *>(this)->set_error(guard.get_error());
        ft_errno = guard.get_error();
        return (guard.get_error());
    }
    error_code = this->_error_code;
    ft_errno = FT_ERR_SUCCESS;
    return (error_code);
}

const char *ft_behavior_action::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_behavior_action::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
