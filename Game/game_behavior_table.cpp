#include "game_behavior_table.hpp"
#include "game_behavior_helpers.hpp"
#include "../Template/move.hpp"

int ft_behavior_table::lock_pair(const ft_behavior_table &first, const ft_behavior_table &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_behavior_table *ordered_first;
    const ft_behavior_table *ordered_second;
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
        const ft_behavior_table *temporary;

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

ft_behavior_table::ft_behavior_table() noexcept
    : _profiles(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_behavior_table::~ft_behavior_table() noexcept
{
    return ;
}

ft_behavior_table::ft_behavior_table(const ft_behavior_table &other) noexcept
    : _profiles(), _error_code(ER_SUCCESS)
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
    this->_profiles = other._profiles;
    this->_error_code = other._error_code;
    this->set_error(this->_profiles.get_error());
    game_behavior_restore_errno(other_guard, entry_errno);
    return ;
}

ft_behavior_table &ft_behavior_table::operator=(const ft_behavior_table &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_behavior_table::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_profiles = other._profiles;
    this->_error_code = other._error_code;
    this->set_error(this->_profiles.get_error());
    game_behavior_restore_errno(this_guard, entry_errno);
    game_behavior_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_behavior_table::ft_behavior_table(ft_behavior_table &&other) noexcept
    : _profiles(ft_move(other._profiles)), _error_code(ER_SUCCESS)
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
    this->_error_code = other._error_code;
    other._profiles.clear();
    other._error_code = ER_SUCCESS;
    this->set_error(this->_profiles.get_error());
    other.set_error(ER_SUCCESS);
    game_behavior_restore_errno(other_guard, entry_errno);
    return ;
}

ft_behavior_table &ft_behavior_table::operator=(ft_behavior_table &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_behavior_table::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_profiles = ft_move(other._profiles);
    this->_error_code = other._error_code;
    other._profiles.clear();
    other._error_code = ER_SUCCESS;
    this->set_error(this->_profiles.get_error());
    other.set_error(ER_SUCCESS);
    game_behavior_restore_errno(this_guard, entry_errno);
    game_behavior_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_map<int, ft_behavior_profile> &ft_behavior_table::get_profiles() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_profiles);
}

const ft_map<int, ft_behavior_profile> &ft_behavior_table::get_profiles() const noexcept
{
    const_cast<ft_behavior_table *>(this)->set_error(ER_SUCCESS);
    return (this->_profiles);
}

void ft_behavior_table::set_profiles(const ft_map<int, ft_behavior_profile> &profiles) noexcept
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
    this->_profiles = profiles;
    this->set_error(this->_profiles.get_error());
    game_behavior_restore_errno(guard, entry_errno);
    return ;
}

int ft_behavior_table::register_profile(const ft_behavior_profile &profile) noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (profile.get_error() != ER_SUCCESS)
    {
        this->set_error(profile.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (profile.get_error());
    }
    identifier = profile.get_profile_id();
    this->_profiles.insert(identifier, profile);
    if (this->_profiles.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_profiles.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (this->_profiles.get_error());
    }
    this->set_error(ER_SUCCESS);
    game_behavior_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_behavior_table::fetch_profile(int profile_id, ft_behavior_profile &profile) const noexcept
{
    int entry_errno;
    const ft_behavior_table *self;
    const Pair<int, ft_behavior_profile> *entry;

    self = this;
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_table *>(self)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    entry = self->_profiles.find(profile_id);
    if (entry == self->_profiles.end())
    {
        const_cast<ft_behavior_table *>(self)->set_error(FT_ERR_NOT_FOUND);
        game_behavior_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    ft_behavior_profile temporary;

    temporary.clone_from_unlocked(entry->value);
    const_cast<ft_behavior_table *>(self)->set_error(temporary.get_error());
    game_behavior_restore_errno(guard, entry_errno);
    if (temporary.get_error() != ER_SUCCESS)
        return (temporary.get_error());
    int destination_errno;

    destination_errno = ft_errno;
    ft_unique_lock<pt_mutex> destination_guard(profile._mutex);
    if (destination_guard.get_error() != ER_SUCCESS)
    {
        profile.set_error(destination_guard.get_error());
        game_behavior_restore_errno(destination_guard, destination_errno);
        return (destination_guard.get_error());
    }
    profile.move_from_unlocked(temporary);
    game_behavior_restore_errno(destination_guard, destination_errno);
    return (profile._error_code);
}

int ft_behavior_table::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_behavior_table *>(this)->set_error(guard.get_error());
        game_behavior_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    game_behavior_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_behavior_table::get_error_str() const noexcept
{
    int error_code;
    int entry_errno;

    entry_errno = ft_errno;
    error_code = this->get_error();
    ft_errno = entry_errno;
    return (ft_strerror(error_code));
}

void ft_behavior_table::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
