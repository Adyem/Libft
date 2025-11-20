#include "ft_behavior_profile.hpp"
#include "game_behavior_helpers.hpp"
#include "../Template/move.hpp"

static void game_behavior_copy_action_vector(const ft_vector<ft_behavior_action> &source,
        ft_vector<ft_behavior_action> &destination)
{
    ft_vector<ft_behavior_action>::const_iterator entry;
    ft_vector<ft_behavior_action>::const_iterator end;

    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

int ft_behavior_profile::lock_pair(const ft_behavior_profile &first, const ft_behavior_profile &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_behavior_profile *ordered_first;
    const ft_behavior_profile *ordered_second;
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
        const ft_behavior_profile *temporary;

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

ft_behavior_profile::ft_behavior_profile() noexcept
    : _profile_id(0), _aggression_weight(0.0), _caution_weight(0.0), _actions(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_behavior_profile::ft_behavior_profile(int profile_id, double aggression_weight, double caution_weight,
        const ft_vector<ft_behavior_action> &actions) noexcept
    : _profile_id(profile_id), _aggression_weight(aggression_weight), _caution_weight(caution_weight),
    _actions(), _error_code(ER_SUCCESS)
{
    game_behavior_copy_action_vector(actions, this->_actions);
    this->set_error(this->_actions.get_error());
    return ;
}

ft_behavior_profile::ft_behavior_profile(const ft_behavior_profile &other) noexcept
    : _profile_id(0), _aggression_weight(0.0), _caution_weight(0.0), _actions(), _error_code(ER_SUCCESS)
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
    this->_profile_id = other._profile_id;
    this->_aggression_weight = other._aggression_weight;
    this->_caution_weight = other._caution_weight;
    game_behavior_copy_action_vector(other._actions, this->_actions);
    this->_error_code = other._error_code;
    this->set_error(this->_actions.get_error());
    game_behavior_restore_errno(other_guard, entry_errno);
    return ;
}

ft_behavior_profile &ft_behavior_profile::operator=(const ft_behavior_profile &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_behavior_profile::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_profile_id = other._profile_id;
    this->_aggression_weight = other._aggression_weight;
    this->_caution_weight = other._caution_weight;
    game_behavior_copy_action_vector(other._actions, this->_actions);
    this->_error_code = other._error_code;
    this->set_error(this->_actions.get_error());
    game_behavior_restore_errno(this_guard, entry_errno);
    game_behavior_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_behavior_profile::ft_behavior_profile(ft_behavior_profile &&other) noexcept
    : _profile_id(0), _aggression_weight(0.0), _caution_weight(0.0), _actions(ft_move(other._actions)),
    _error_code(ER_SUCCESS)
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
    this->_profile_id = other._profile_id;
    this->_aggression_weight = other._aggression_weight;
    this->_caution_weight = other._caution_weight;
    this->_error_code = other._error_code;
    other._profile_id = 0;
    other._aggression_weight = 0.0;
    other._caution_weight = 0.0;
    other._actions.clear();
    other._error_code = ER_SUCCESS;
    this->set_error(this->_actions.get_error());
    other.set_error(ER_SUCCESS);
    game_behavior_restore_errno(other_guard, entry_errno);
    return ;
}

ft_behavior_profile &ft_behavior_profile::operator=(ft_behavior_profile &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_behavior_profile::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_profile_id = other._profile_id;
    this->_aggression_weight = other._aggression_weight;
    this->_caution_weight = other._caution_weight;
    this->_actions = ft_move(other._actions);
    this->_error_code = other._error_code;
    other._profile_id = 0;
    other._aggression_weight = 0.0;
    other._caution_weight = 0.0;
    other._actions.clear();
    other._error_code = ER_SUCCESS;
    this->set_error(this->_actions.get_error());
    other.set_error(ER_SUCCESS);
    game_behavior_restore_errno(this_guard, entry_errno);
    game_behavior_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_behavior_profile::get_profile_id() const noexcept
{
    int entry_errno;
    int profile_id;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_profile *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (0);
    }
    profile_id = this->_profile_id;
    const_cast<ft_behavior_profile *>(this)->set_error(this->_error_code);
    game_behavior_restore_errno(guard, entry_errno);
    return (profile_id);
}

void ft_behavior_profile::set_profile_id(int profile_id) noexcept
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
    this->_profile_id = profile_id;
    this->set_error(ER_SUCCESS);
    game_behavior_restore_errno(guard, entry_errno);
    return ;
}

double ft_behavior_profile::get_aggression_weight() const noexcept
{
    int entry_errno;
    double aggression_weight;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_profile *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (0.0);
    }
    aggression_weight = this->_aggression_weight;
    const_cast<ft_behavior_profile *>(this)->set_error(this->_error_code);
    game_behavior_restore_errno(guard, entry_errno);
    return (aggression_weight);
}

void ft_behavior_profile::set_aggression_weight(double aggression_weight) noexcept
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
    this->_aggression_weight = aggression_weight;
    this->set_error(ER_SUCCESS);
    game_behavior_restore_errno(guard, entry_errno);
    return ;
}

double ft_behavior_profile::get_caution_weight() const noexcept
{
    int entry_errno;
    double caution_weight;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_profile *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (0.0);
    }
    caution_weight = this->_caution_weight;
    const_cast<ft_behavior_profile *>(this)->set_error(this->_error_code);
    game_behavior_restore_errno(guard, entry_errno);
    return (caution_weight);
}

void ft_behavior_profile::set_caution_weight(double caution_weight) noexcept
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
    this->_caution_weight = caution_weight;
    this->set_error(ER_SUCCESS);
    game_behavior_restore_errno(guard, entry_errno);
    return ;
}

ft_vector<ft_behavior_action> &ft_behavior_profile::get_actions() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_actions);
}

const ft_vector<ft_behavior_action> &ft_behavior_profile::get_actions() const noexcept
{
    const_cast<ft_behavior_profile *>(this)->set_error(ER_SUCCESS);
    return (this->_actions);
}

void ft_behavior_profile::set_actions(const ft_vector<ft_behavior_action> &actions) noexcept
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
    game_behavior_copy_action_vector(actions, this->_actions);
    this->set_error(this->_actions.get_error());
    game_behavior_restore_errno(guard, entry_errno);
    return ;
}

int ft_behavior_profile::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_profile *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    game_behavior_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_behavior_profile::get_error_str() const noexcept
{
    int error_code;
    int entry_errno;

    entry_errno = ft_errno;
    error_code = this->get_error();
    ft_errno = entry_errno;
    return (ft_strerror(error_code));
}

void ft_behavior_profile::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
