#include "game_behavior_table.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"

thread_local ft_operation_error_stack ft_behavior_table::_operation_errors = {{}, {}, 0};

void ft_behavior_table::record_operation_error_unlocked(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(ft_behavior_table::_operation_errors,
            error_code, operation_id);
    return ;
}

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

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_behavior_table::record_operation_error_unlocked(single_guard.get_error());
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_behavior_table::record_operation_error_unlocked(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
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

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_behavior_table::record_operation_error_unlocked(lower_guard.get_error());
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
            ft_behavior_table::record_operation_error_unlocked(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_behavior_table::record_operation_error_unlocked(upper_guard.get_error());
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_behavior_table::ft_behavior_table() noexcept
    : _profiles(), _error_code(FT_ERR_SUCCESS), _mutex()
{
    return ;
}

ft_behavior_table::~ft_behavior_table() noexcept
{
    return ;
}

int ft_behavior_table::clone_profiles_from(const ft_behavior_table &other) noexcept
{
    size_t profile_count;
    ft_map<int, ft_behavior_profile> profiles_copy(other._profiles.capacity());
    const Pair<int, ft_behavior_profile> *other_end;
    const Pair<int, ft_behavior_profile> *entry;

    profile_count = other._profiles.size();
    if (other._profiles.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other._profiles.get_error());
        return (other._profiles.get_error());
    }
    if (profiles_copy.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(profiles_copy.get_error());
        return (profiles_copy.get_error());
    }
    other_end = other._profiles.end();
    if (other_end == ft_nullptr && profile_count != 0)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    if (profile_count == 0)
        entry = other_end;
    else
        entry = other_end - profile_count;
    while (entry != other_end)
    {
        ft_behavior_profile profile_copy;
        ft_vector<ft_behavior_action>::const_iterator action_entry;
        ft_vector<ft_behavior_action>::const_iterator action_end;

        profile_copy._profile_id = entry->value._profile_id;
        profile_copy._aggression_weight = entry->value._aggression_weight;
        profile_copy._caution_weight = entry->value._caution_weight;
        profile_copy._actions.clear();
        action_entry = entry->value._actions.begin();
        action_end = entry->value._actions.end();
        while (action_entry != action_end)
        {
            profile_copy._actions.push_back(*action_entry);
            ++action_entry;
        }
        profile_copy._error_code = entry->value._error_code;
        profile_copy.set_error(profile_copy._actions.get_error());
        profiles_copy.insert(entry->key, ft_move(profile_copy));
        if (profiles_copy.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(profiles_copy.get_error());
            return (profiles_copy.get_error());
        }
        ++entry;
    }
    this->_profiles = ft_move(profiles_copy);
    ft_behavior_table::record_operation_error_unlocked(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

ft_behavior_table::ft_behavior_table(const ft_behavior_table &other) noexcept
    : _profiles(), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_error_code = other._error_code;
    if (this->clone_profiles_from(other) != FT_ERR_SUCCESS)
        return ;
    ft_behavior_table::record_operation_error_unlocked(FT_ERR_SUCCESS);
    return ;
}

ft_behavior_table &ft_behavior_table::operator=(const ft_behavior_table &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_behavior_table::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_error_code = other._error_code;
    {
        int clone_error;

        clone_error = this->clone_profiles_from(other);
        if (clone_error != FT_ERR_SUCCESS)
            return (*this);
    }
    return (*this);
}

ft_behavior_table::ft_behavior_table(ft_behavior_table &&other) noexcept
    : _profiles(), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_error_code = other._error_code;
    if (this->clone_profiles_from(other) != FT_ERR_SUCCESS)
        return ;
    other._profiles.clear();
    other._error_code = FT_ERR_SUCCESS;
    other.set_error(FT_ERR_SUCCESS);
    return ;
}

ft_behavior_table &ft_behavior_table::operator=(ft_behavior_table &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_behavior_table::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_error_code = other._error_code;
    if (this->clone_profiles_from(other) != FT_ERR_SUCCESS)
    {
        return (*this);
    }
    other._profiles.clear();
    other._error_code = FT_ERR_SUCCESS;
    other.set_error(FT_ERR_SUCCESS);
    return (*this);
}

ft_map<int, ft_behavior_profile> &ft_behavior_table::get_profiles() noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    ft_behavior_table::record_operation_error_unlocked(FT_ERR_SUCCESS);
    return (this->_profiles);
}

const ft_map<int, ft_behavior_profile> &ft_behavior_table::get_profiles() const noexcept
{
    const_cast<ft_behavior_table *>(this)->set_error(FT_ERR_SUCCESS);
    ft_behavior_table::record_operation_error_unlocked(FT_ERR_SUCCESS);
    return (this->_profiles);
}

void ft_behavior_table::set_profiles(const ft_map<int, ft_behavior_profile> &profiles) noexcept
{
    ft_unique_lock<pt_mutex> guard;

    guard = ft_unique_lock<pt_mutex>(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_profiles = profiles;
    this->set_error(this->_profiles.get_error());
    return ;
}

int ft_behavior_table::register_profile(const ft_behavior_profile &profile) noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (this->_profiles.find(profile.get_profile_id()) != this->_profiles.end())
    {
        this->set_error(FT_ERR_ALREADY_EXISTS);
        return (FT_ERR_ALREADY_EXISTS);
    }
    if (profile.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(profile.get_error());
        return (profile.get_error());
    }
    identifier = profile.get_profile_id();
    this->_profiles.insert(identifier, profile);
    if (this->_profiles.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_profiles.get_error());
        return (this->_profiles.get_error());
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_behavior_table::fetch_profile(int profile_id, ft_behavior_profile &profile) const noexcept
{
    const ft_behavior_table *self;
    const Pair<int, ft_behavior_profile> *entry;

    self = this;
    ft_unique_lock<pt_mutex> guard(self->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_table *>(self)->set_error(guard.get_error());
        return (guard.get_error());
    }
    entry = self->_profiles.find(profile_id);
    if (entry == self->_profiles.end())
    {
        const_cast<ft_behavior_table *>(self)->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    ft_unique_lock<pt_mutex> entry_guard(entry->value._mutex);
    if (entry_guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_table *>(self)->set_error(entry_guard.get_error());
        return (entry_guard.get_error());
    }
    ft_behavior_profile entry_profile;
    entry_profile.clone_from_unlocked(entry->value);
    if (entry_profile.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_table *>(self)->set_error(entry_profile.get_error());
        return (entry_profile.get_error());
    }
    ft_unique_lock<pt_mutex> destination_guard(profile._mutex);
    if (destination_guard.get_error() != FT_ERR_SUCCESS)
    {
        profile.set_error(destination_guard.get_error());
        return (destination_guard.get_error());
    }
    profile.move_from_unlocked(entry_profile);
    const_cast<ft_behavior_table *>(self)->set_error(self->_error_code);
    return (profile._error_code);
}

int ft_behavior_table::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_behavior_table *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    ft_behavior_table::record_operation_error_unlocked(error_code);
    return (error_code);
}

const char *ft_behavior_table::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_behavior_table::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_behavior_table::record_operation_error_unlocked(error_code);
    return ;
}
