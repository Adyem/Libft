#include "game_quest.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_quest_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_quest_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    (void)entry_errno;
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_quest::lock_pair(const ft_quest &first, const ft_quest &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_quest *ordered_first;
    const ft_quest *ordered_second;
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
        const ft_quest *temporary;

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
        game_quest_sleep_backoff();
    }
}

ft_quest::ft_quest() noexcept
    : _id(0), _phases(0), _current_phase(0), _description(),
      _objective(), _reward_experience(0), _reward_items(),
      _error(FT_ER_SUCCESSS), _mutex()
{
    if (this->_description.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_description.get_error());
        return ;
    }
    if (this->_objective.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_objective.get_error());
        return ;
    }
    if (this->_reward_items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_reward_items.get_error());
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

ft_quest::~ft_quest() noexcept
{
    return ;
}

ft_quest::ft_quest(const ft_quest &other) noexcept
    : _id(0), _phases(0), _current_phase(0), _description(),
      _objective(), _reward_experience(0), _reward_items(),
      _error(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_id = other._id;
    this->_phases = other._phases;
    this->_current_phase = other._current_phase;
    this->_description = other._description;
    if (this->_description.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_description.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    if (other._description.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._description.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_objective = other._objective;
    if (this->_objective.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_objective.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    if (other._objective.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._objective.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_reward_experience = other._reward_experience;
    ft_vector<ft_sharedptr<ft_item> > temporary_items;
    size_t index;
    size_t count;

    temporary_items = ft_vector<ft_sharedptr<ft_item> >();
    if (temporary_items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(temporary_items.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    count = other._reward_items.size();
    if (other._reward_items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._reward_items.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    index = 0;
    while (index < count)
    {
        const ft_sharedptr<ft_item> &item_ptr = other._reward_items[index];

        if (other._reward_items.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(other._reward_items.get_error());
            game_quest_restore_errno(other_guard, entry_errno);
            return ;
        }
        if (!item_ptr)
        {
            this->set_error(FT_ERR_INVALID_POINTER);
            game_quest_restore_errno(other_guard, entry_errno);
            return ;
        }
        if (item_ptr.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(item_ptr.get_error());
            game_quest_restore_errno(other_guard, entry_errno);
            return ;
        }
        if (item_ptr->get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(item_ptr->get_error());
            game_quest_restore_errno(other_guard, entry_errno);
            return ;
        }
        temporary_items.push_back(item_ptr);
        if (temporary_items.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temporary_items.get_error());
            game_quest_restore_errno(other_guard, entry_errno);
            return ;
        }
        index++;
    }
    this->_reward_items = ft_move(temporary_items);
    if (this->_reward_items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_reward_items.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->set_error(other._error);
    game_quest_restore_errno(other_guard, entry_errno);
    return ;
}

ft_quest &ft_quest::operator=(const ft_quest &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_quest::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_phases = other._phases;
    this->_current_phase = other._current_phase;
    this->_description = other._description;
    if (this->_description.get_error() != FT_ER_SUCCESSS)
    {
        int error_code;

        error_code = this->_description.get_error();
        this->set_error(error_code);
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    if (other._description.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._description.get_error());
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_objective = other._objective;
    if (this->_objective.get_error() != FT_ER_SUCCESSS)
    {
        int error_code;

        error_code = this->_objective.get_error();
        this->set_error(error_code);
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    if (other._objective.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._objective.get_error());
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_reward_experience = other._reward_experience;
    ft_vector<ft_sharedptr<ft_item> > temporary_items;
    size_t index;
    size_t count;

    temporary_items = ft_vector<ft_sharedptr<ft_item> >();
    if (temporary_items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(temporary_items.get_error());
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    count = other._reward_items.size();
    if (other._reward_items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._reward_items.get_error());
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    index = 0;
    while (index < count)
    {
        const ft_sharedptr<ft_item> &item_ptr = other._reward_items[index];

        if (other._reward_items.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(other._reward_items.get_error());
            game_quest_restore_errno(this_guard, entry_errno);
            game_quest_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        if (!item_ptr)
        {
            this->set_error(FT_ERR_INVALID_POINTER);
            game_quest_restore_errno(this_guard, entry_errno);
            game_quest_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        if (item_ptr.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(item_ptr.get_error());
            game_quest_restore_errno(this_guard, entry_errno);
            game_quest_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        if (item_ptr->get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(item_ptr->get_error());
            game_quest_restore_errno(this_guard, entry_errno);
            game_quest_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        temporary_items.push_back(item_ptr);
        if (temporary_items.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temporary_items.get_error());
            game_quest_restore_errno(this_guard, entry_errno);
            game_quest_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        index++;
    }
    this->_reward_items = ft_move(temporary_items);
    if (this->_reward_items.get_error() != FT_ER_SUCCESSS)
    {
        int error_code;

        error_code = this->_reward_items.get_error();
        this->set_error(error_code);
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->set_error(other._error);
    game_quest_restore_errno(this_guard, entry_errno);
    game_quest_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_quest::ft_quest(ft_quest &&other) noexcept
    : _id(0), _phases(0), _current_phase(0), _description(),
      _objective(), _reward_experience(0), _reward_items(),
      _error(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_id = other._id;
    this->_phases = other._phases;
    this->_current_phase = other._current_phase;
    this->_description = ft_move(other._description);
    if (this->_description.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_description.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_objective = ft_move(other._objective);
    if (this->_objective.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_objective.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_reward_experience = other._reward_experience;
    this->_reward_items = ft_move(other._reward_items);
    if (this->_reward_items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_reward_items.get_error());
        game_quest_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->set_error(other._error);
    other._id = 0;
    other._phases = 0;
    other._current_phase = 0;
    other._reward_experience = 0;
    other.set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(other_guard, entry_errno);
    return ;
}

ft_quest &ft_quest::operator=(ft_quest &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_quest::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_phases = other._phases;
    this->_current_phase = other._current_phase;
    this->_description = ft_move(other._description);
    if (this->_description.get_error() != FT_ER_SUCCESSS)
    {
        int error_code;

        error_code = this->_description.get_error();
        this->set_error(error_code);
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_objective = ft_move(other._objective);
    if (this->_objective.get_error() != FT_ER_SUCCESSS)
    {
        int error_code;

        error_code = this->_objective.get_error();
        this->set_error(error_code);
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_reward_experience = other._reward_experience;
    this->_reward_items = ft_move(other._reward_items);
    if (this->_reward_items.get_error() != FT_ER_SUCCESSS)
    {
        int error_code;

        error_code = this->_reward_items.get_error();
        this->set_error(error_code);
        game_quest_restore_errno(this_guard, entry_errno);
        game_quest_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->set_error(other._error);
    other._id = 0;
    other._phases = 0;
    other._current_phase = 0;
    other._reward_experience = 0;
    other.set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(this_guard, entry_errno);
    game_quest_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_quest::get_id() const noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (0);
    }
    identifier = this->_id;
    const_cast<ft_quest *>(this)->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return (identifier);
}

void ft_quest::set_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->_id = id;
    this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return ;
}

int ft_quest::get_phases() const noexcept
{
    int entry_errno;
    int phases_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (0);
    }
    phases_value = this->_phases;
    const_cast<ft_quest *>(this)->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return (phases_value);
}

void ft_quest::set_phases(int phases) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (phases < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->_phases = phases;
    if (this->_current_phase > this->_phases)
        this->_current_phase = this->_phases;
    this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return ;
}

int ft_quest::get_current_phase() const noexcept
{
    int entry_errno;
    int phase_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (0);
    }
    phase_value = this->_current_phase;
    const_cast<ft_quest *>(this)->set_error(this->_error);
    game_quest_restore_errno(guard, entry_errno);
    return (phase_value);
}

void ft_quest::set_current_phase(int phase) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (phase < 0 || phase > this->_phases)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->_current_phase = phase;
    this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return ;
}

const ft_string &ft_quest::get_description() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (this->_description);
    }
    if (this->_description.get_error() != FT_ER_SUCCESSS)
        const_cast<ft_quest *>(this)->set_error(this->_description.get_error());
    else
        const_cast<ft_quest *>(this)->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return (this->_description);
}

void ft_quest::set_description(const ft_string &description) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (description.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(description.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->_description = description;
    if (this->_description.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_description.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return ;
}

const ft_string &ft_quest::get_objective() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (this->_objective);
    }
    if (this->_objective.get_error() != FT_ER_SUCCESSS)
        const_cast<ft_quest *>(this)->set_error(this->_objective.get_error());
    else
        const_cast<ft_quest *>(this)->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return (this->_objective);
}

void ft_quest::set_objective(const ft_string &objective) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (objective.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(objective.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->_objective = objective;
    if (this->_objective.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_objective.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return ;
}

int ft_quest::get_reward_experience() const noexcept
{
    int entry_errno;
    int reward_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (0);
    }
    reward_value = this->_reward_experience;
    const_cast<ft_quest *>(this)->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return (reward_value);
}

void ft_quest::set_reward_experience(int experience) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (experience < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->_reward_experience = experience;
    this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return ;
}

ft_vector<ft_sharedptr<ft_item> > &ft_quest::get_reward_items() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (this->_reward_items);
    }
    if (this->_reward_items.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_reward_items.get_error());
    else
        this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return (this->_reward_items);
}

const ft_vector<ft_sharedptr<ft_item> > &ft_quest::get_reward_items() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (this->_reward_items);
    }
    if (this->_reward_items.get_error() != FT_ER_SUCCESSS)
        const_cast<ft_quest *>(this)->set_error(this->_reward_items.get_error());
    else
        const_cast<ft_quest *>(this)->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return (this->_reward_items);
}

void ft_quest::set_reward_items(const ft_vector<ft_sharedptr<ft_item> > &items) noexcept
{
    int entry_errno;
    size_t index;
    size_t count;
    ft_vector<ft_sharedptr<ft_item> > temporary;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(items.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    temporary = ft_vector<ft_sharedptr<ft_item> >();
    if (temporary.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(temporary.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    index = 0;
    count = items.size();
    while (index < count)
    {
        const ft_sharedptr<ft_item> &item_ptr = items[index];

        if (items.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(items.get_error());
            game_quest_restore_errno(guard, entry_errno);
            return ;
        }
        if (!item_ptr)
        {
            this->set_error(FT_ERR_INVALID_POINTER);
            game_quest_restore_errno(guard, entry_errno);
            return ;
        }
        if (item_ptr.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(item_ptr.get_error());
            game_quest_restore_errno(guard, entry_errno);
            return ;
        }
        if (item_ptr->get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(item_ptr->get_error());
            game_quest_restore_errno(guard, entry_errno);
            return ;
        }
        temporary.push_back(item_ptr);
        if (temporary.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temporary.get_error());
            game_quest_restore_errno(guard, entry_errno);
            return ;
        }
        index++;
    }
    this->_reward_items = ft_move(temporary);
    if (this->_reward_items.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_reward_items.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return ;
}

bool ft_quest::is_complete() const noexcept
{
    int entry_errno;
    bool complete;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (false);
    }
    complete = this->_current_phase >= this->_phases;
    const_cast<ft_quest *>(this)->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return (complete);
}

void ft_quest::advance_phase() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (this->_phases <= 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->_current_phase = 0;
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    if (this->_current_phase >= this->_phases)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        game_quest_restore_errno(guard, entry_errno);
        return ;
    }
    this->_current_phase += 1;
    this->set_error(FT_ER_SUCCESSS);
    game_quest_restore_errno(guard, entry_errno);
    return ;
}

int ft_quest::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_quest *>(this)->set_error(error_code);
    game_quest_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_quest::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_quest *>(this)->set_error(guard.get_error());
        game_quest_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_quest *>(this)->set_error(error_code);
    game_quest_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_quest::set_error(int err) const noexcept
{
    this->_error = err;
    ft_errno = err;
    return ;
}
