#include "game_achievement.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_achievement_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_achievement_unlock_guard(ft_unique_lock<pt_mutex> &guard)
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

static void game_goal_sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

static void game_goal_unlock_guard(ft_unique_lock<pt_mutex> &guard) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_goal::lock_pair(const ft_goal &first,
        const ft_goal &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_goal *ordered_first;
    const ft_goal *ordered_second;
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
        const ft_goal *temporary;

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
        game_goal_sleep_backoff();
    }
}

ft_goal::ft_goal() noexcept
    : _target(0), _progress(0), _error(FT_ERR_SUCCESS), _mutex()
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_goal::~ft_goal() noexcept
{
    return ;
}

ft_goal::ft_goal(const ft_goal &other) noexcept
    : _target(0), _progress(0), _error(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_target = other._target;
    this->_progress = other._progress;
    this->_error = other._error;
    this->set_error(other._error);
    game_goal_unlock_guard(other_guard);
    return ;
}

ft_goal &ft_goal::operator=(const ft_goal &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_goal::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_target = other._target;
    this->_progress = other._progress;
    this->_error = other._error;
    this->set_error(other._error);
    game_goal_unlock_guard(this_guard);
    game_goal_unlock_guard(other_guard);
    return (*this);
}

ft_goal::ft_goal(ft_goal &&other) noexcept
    : _target(0), _progress(0), _error(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_target = other._target;
    this->_progress = other._progress;
    this->_error = other._error;
    other._target = 0;
    other._progress = 0;
    other._error = FT_ERR_SUCCESS;
    this->set_error(this->_error);
    other.set_error(FT_ERR_SUCCESS);
    game_goal_unlock_guard(other_guard);
    return ;
}

ft_goal &ft_goal::operator=(ft_goal &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_goal::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_target = other._target;
    this->_progress = other._progress;
    this->_error = other._error;
    other._target = 0;
    other._progress = 0;
    other._error = FT_ERR_SUCCESS;
    this->set_error(this->_error);
    other.set_error(FT_ERR_SUCCESS);
    game_goal_unlock_guard(this_guard);
    game_goal_unlock_guard(other_guard);
    return (*this);
}

int ft_goal::get_target() const noexcept
{
    int target_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_goal *>(this)->set_error(guard.get_error());
        return (0);
    }
    target_value = this->_target;
    const_cast<ft_goal *>(this)->set_error(FT_ERR_SUCCESS);
    game_goal_unlock_guard(guard);
    return (target_value);
}

void ft_goal::set_target(int target) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_target = target;
    this->set_error(FT_ERR_SUCCESS);
    game_goal_unlock_guard(guard);
    return ;
}

int ft_goal::get_progress() const noexcept
{
    int progress_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_goal *>(this)->set_error(guard.get_error());
        return (0);
    }
    progress_value = this->_progress;
    const_cast<ft_goal *>(this)->set_error(FT_ERR_SUCCESS);
    game_goal_unlock_guard(guard);
    return (progress_value);
}

void ft_goal::set_progress(int value) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_progress = value;
    this->set_error(FT_ERR_SUCCESS);
    game_goal_unlock_guard(guard);
    return ;
}

void ft_goal::add_progress(int delta) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_progress += delta;
    this->set_error(FT_ERR_SUCCESS);
    game_goal_unlock_guard(guard);
    return ;
}

int ft_goal::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_goal *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_goal *>(this)->set_error(error_code);
    game_goal_unlock_guard(guard);
    return (error_code);
}

const char *ft_goal::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_goal *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_goal *>(this)->set_error(error_code);
    game_goal_unlock_guard(guard);
    return (ft_strerror(error_code));
}

void ft_goal::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error = error;
    return ;
}

int ft_achievement::lock_pair(const ft_achievement &first,
        const ft_achievement &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_achievement *ordered_first;
    const ft_achievement *ordered_second;
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
        const ft_achievement *temporary;

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
        game_achievement_sleep_backoff();
    }
}

ft_achievement::ft_achievement() noexcept
    : _id(0), _goals(), _error(FT_ERR_SUCCESS), _mutex()
{
    if (this->_goals.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_achievement::~ft_achievement() noexcept
{
    return ;
}

ft_achievement::ft_achievement(const ft_achievement &other) noexcept
    : _id(0), _goals(), _error(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_id = other._id;
    this->_goals = other._goals;
    if (this->_goals.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        return ;
    }
    this->_error = other._error;
    this->set_error(this->_error);
    game_achievement_unlock_guard(other_guard);
    return ;
}

ft_achievement &ft_achievement::operator=(const ft_achievement &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_achievement::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_goals = other._goals;
    if (this->_goals.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        game_achievement_unlock_guard(this_guard);
        game_achievement_unlock_guard(other_guard);
        return (*this);
    }
    this->_error = other._error;
    this->set_error(this->_error);
    game_achievement_unlock_guard(this_guard);
    game_achievement_unlock_guard(other_guard);
    return (*this);
}

ft_achievement::ft_achievement(ft_achievement &&other) noexcept
    : _id(0), _goals(), _error(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_id = other._id;
    this->_goals = ft_move(other._goals);
    if (this->_goals.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        return ;
    }
    this->_error = other._error;
    other._id = 0;
    other._error = FT_ERR_SUCCESS;
    other._goals.clear();
    if (other._goals.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other._goals.get_error());
        return ;
    }
    this->set_error(this->_error);
    other.set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(other_guard);
    return ;
}

ft_achievement &ft_achievement::operator=(ft_achievement &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_achievement::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_goals = ft_move(other._goals);
    if (this->_goals.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        game_achievement_unlock_guard(this_guard);
        game_achievement_unlock_guard(other_guard);
        return (*this);
    }
    this->_error = other._error;
    other._id = 0;
    other._error = FT_ERR_SUCCESS;
    other._goals.clear();
    if (other._goals.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other._goals.get_error());
        game_achievement_unlock_guard(this_guard);
        game_achievement_unlock_guard(other_guard);
        return (*this);
    }
    this->set_error(this->_error);
    other.set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(this_guard);
    game_achievement_unlock_guard(other_guard);
    return (*this);
}

int ft_achievement::get_id() const noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        return (0);
    }
    identifier = this->_id;
    const_cast<ft_achievement *>(this)->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return (identifier);
}

void ft_achievement::set_id(int id) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_id = id;
    this->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return ;
}

ft_map<int, ft_goal> &ft_achievement::get_goals() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (this->_goals);
    }
    this->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return (this->_goals);
}

const ft_map<int, ft_goal> &ft_achievement::get_goals() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        return (this->_goals);
    }
    const_cast<ft_achievement *>(this)->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return (this->_goals);
}

void ft_achievement::set_goals(const ft_map<int, ft_goal> &goals) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_goals = goals;
    if (this->_goals.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        game_achievement_unlock_guard(guard);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return ;
}

int ft_achievement::get_goal(int id) const noexcept
{
    int goal_value;
    const Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    goal_value = entry->value.get_target();
    if (entry->value.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(entry->value.get_error());
        return (0);
    }
    const_cast<ft_achievement *>(this)->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return (goal_value);
}

void ft_achievement::set_goal(int id, int goal) noexcept
{
    Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        ft_goal new_goal;

        new_goal.set_target(goal);
        if (new_goal.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(new_goal.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
        new_goal.set_progress(0);
        if (new_goal.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(new_goal.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
    }
    else
    {
        entry->value.set_target(goal);
        if (entry->value.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(entry->value.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return ;
}

int ft_achievement::get_progress(int id) const noexcept
{
    int progress_value;
    const Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        return (0);
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    progress_value = entry->value.get_progress();
    if (entry->value.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(entry->value.get_error());
        return (0);
    }
    const_cast<ft_achievement *>(this)->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return (progress_value);
}

void ft_achievement::set_progress(int id, int progress) noexcept
{
    Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        ft_goal new_goal;

        new_goal.set_target(0);
        if (new_goal.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(new_goal.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
        new_goal.set_progress(progress);
        if (new_goal.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(new_goal.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
    }
    else
    {
        entry->value.set_progress(progress);
        if (entry->value.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(entry->value.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return ;
}

void ft_achievement::add_progress(int id, int value) noexcept
{
    Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        ft_goal new_goal;

        new_goal.set_target(0);
        if (new_goal.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(new_goal.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
        new_goal.set_progress(value);
        if (new_goal.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(new_goal.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
    }
    else
    {
        entry->value.add_progress(value);
        if (entry->value.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(entry->value.get_error());
            game_achievement_unlock_guard(guard);
            return ;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return ;
}

bool ft_achievement::is_goal_complete(int id) const noexcept
{
    const Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        return (false);
    }
    entry = this->_goals.find(id);
    if (entry == this->_goals.end())
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    int progress_value;
    int target_value;

    progress_value = entry->value.get_progress();
    if (entry->value.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(entry->value.get_error());
        return (false);
    }
    target_value = entry->value.get_target();
    if (entry->value.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(entry->value.get_error());
        return (false);
    }
    if (progress_value < target_value)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_SUCCESS);
        return (false);
    }
    const_cast<ft_achievement *>(this)->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return (true);
}

bool ft_achievement::is_complete() const noexcept
{
    const Pair<int, ft_goal> *goal_ptr;
    const Pair<int, ft_goal> *goal_end;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        return (false);
    }
    goal_ptr = this->_goals.end() - this->_goals.size();
    goal_end = this->_goals.end();
    while (goal_ptr != goal_end)
    {
        int progress_value;
        int target_value;

        progress_value = goal_ptr->value.get_progress();
        if (goal_ptr->value.get_error() != FT_ERR_SUCCESS)
        {
            const_cast<ft_achievement *>(this)->set_error(goal_ptr->value.get_error());
            return (false);
        }
        target_value = goal_ptr->value.get_target();
        if (goal_ptr->value.get_error() != FT_ERR_SUCCESS)
        {
            const_cast<ft_achievement *>(this)->set_error(goal_ptr->value.get_error());
            return (false);
        }
        if (progress_value < target_value)
        {
            const_cast<ft_achievement *>(this)->set_error(FT_ERR_SUCCESS);
            return (false);
        }
        ++goal_ptr;
    }
    const_cast<ft_achievement *>(this)->set_error(FT_ERR_SUCCESS);
    game_achievement_unlock_guard(guard);
    return (true);
}

int ft_achievement::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_achievement *>(this)->set_error(error_code);
    game_achievement_unlock_guard(guard);
    return (error_code);
}

const char *ft_achievement::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_achievement *>(this)->set_error(error_code);
    game_achievement_unlock_guard(guard);
    return (ft_strerror(error_code));
}

void ft_achievement::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
