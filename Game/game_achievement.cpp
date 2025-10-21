#include "game_achievement.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_achievement_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_achievement_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
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
        const ft_achievement *temporary;

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
        game_achievement_sleep_backoff();
    }
}

ft_achievement::ft_achievement() noexcept
    : _id(0), _goals(), _error(ER_SUCCESS), _mutex()
{
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

ft_achievement::~ft_achievement() noexcept
{
    return ;
}

ft_achievement::ft_achievement(const ft_achievement &other) noexcept
    : _id(0), _goals(), _error(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_achievement_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_id = other._id;
    this->_goals = other._goals;
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        game_achievement_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_error = other._error;
    this->set_error(other._error);
    game_achievement_restore_errno(other_guard, entry_errno);
    return ;
}

ft_achievement &ft_achievement::operator=(const ft_achievement &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_achievement::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_goals = other._goals;
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        game_achievement_restore_errno(this_guard, entry_errno);
        game_achievement_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_error = other._error;
    this->set_error(other._error);
    game_achievement_restore_errno(this_guard, entry_errno);
    game_achievement_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_achievement::ft_achievement(ft_achievement &&other) noexcept
    : _id(0), _goals(), _error(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_achievement_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_id = other._id;
    this->_goals = ft_move(other._goals);
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        game_achievement_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_error = other._error;
    other._id = 0;
    other._error = ER_SUCCESS;
    other._goals.clear();
    if (other._goals.get_error() != ER_SUCCESS)
    {
        this->set_error(other._goals.get_error());
        game_achievement_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    game_achievement_restore_errno(other_guard, entry_errno);
    return ;
}

ft_achievement &ft_achievement::operator=(ft_achievement &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_achievement::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_goals = ft_move(other._goals);
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        game_achievement_restore_errno(this_guard, entry_errno);
        game_achievement_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_error = other._error;
    other._id = 0;
    other._error = ER_SUCCESS;
    other._goals.clear();
    if (other._goals.get_error() != ER_SUCCESS)
    {
        this->set_error(other._goals.get_error());
        game_achievement_restore_errno(this_guard, entry_errno);
        game_achievement_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    game_achievement_restore_errno(this_guard, entry_errno);
    game_achievement_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_achievement::get_id() const noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (0);
    }
    identifier = this->_id;
    const_cast<ft_achievement *>(this)->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return (identifier);
}

void ft_achievement::set_id(int id) noexcept
{
    int entry_errno;

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return ;
    }
    this->_id = id;
    this->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return ;
}

ft_map<int, ft_goal> &ft_achievement::get_goals() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (this->_goals);
    }
    this->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return (this->_goals);
}

const ft_map<int, ft_goal> &ft_achievement::get_goals() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (this->_goals);
    }
    const_cast<ft_achievement *>(this)->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return (this->_goals);
}

void ft_achievement::set_goals(const ft_map<int, ft_goal> &goals) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return ;
    }
    this->_goals = goals;
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return ;
}

int ft_achievement::get_goal(int id) const noexcept
{
    int entry_errno;
    int goal_value;
    const Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (0);
    }
    entry = this->_goals.find(id);
    if (!entry)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_achievement_restore_errno(guard, entry_errno);
        return (0);
    }
    goal_value = entry->value.goal;
    const_cast<ft_achievement *>(this)->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return (goal_value);
}

void ft_achievement::set_goal(int id, int goal) noexcept
{
    int entry_errno;
    Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return ;
    }
    entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal;

        new_goal.goal = goal;
        new_goal.progress = 0;
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            game_achievement_restore_errno(guard, entry_errno);
            return ;
        }
    }
    else
        entry->value.goal = goal;
    this->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return ;
}

int ft_achievement::get_progress(int id) const noexcept
{
    int entry_errno;
    int progress_value;
    const Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (0);
    }
    entry = this->_goals.find(id);
    if (!entry)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_achievement_restore_errno(guard, entry_errno);
        return (0);
    }
    progress_value = entry->value.progress;
    const_cast<ft_achievement *>(this)->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return (progress_value);
}

void ft_achievement::set_progress(int id, int progress) noexcept
{
    int entry_errno;
    Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return ;
    }
    entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal;

        new_goal.goal = 0;
        new_goal.progress = progress;
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            game_achievement_restore_errno(guard, entry_errno);
            return ;
        }
    }
    else
        entry->value.progress = progress;
    this->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return ;
}

void ft_achievement::add_progress(int id, int value) noexcept
{
    int entry_errno;
    Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return ;
    }
    entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal;

        new_goal.goal = 0;
        new_goal.progress = value;
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            game_achievement_restore_errno(guard, entry_errno);
            return ;
        }
    }
    else
        entry->value.progress += value;
    this->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return ;
}

bool ft_achievement::is_goal_complete(int id) const noexcept
{
    int entry_errno;
    const Pair<int, ft_goal> *entry;

    if (id < 0)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (false);
    }
    entry = this->_goals.find(id);
    if (!entry)
    {
        const_cast<ft_achievement *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_achievement_restore_errno(guard, entry_errno);
        return (false);
    }
    if (entry->value.progress < entry->value.goal)
    {
        const_cast<ft_achievement *>(this)->set_error(ER_SUCCESS);
        game_achievement_restore_errno(guard, entry_errno);
        return (false);
    }
    const_cast<ft_achievement *>(this)->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return (true);
}

bool ft_achievement::is_complete() const noexcept
{
    int entry_errno;
    const Pair<int, ft_goal> *goal_ptr;
    const Pair<int, ft_goal> *goal_end;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (false);
    }
    goal_ptr = this->_goals.end() - this->_goals.size();
    goal_end = this->_goals.end();
    while (goal_ptr != goal_end)
    {
        if (goal_ptr->value.progress < goal_ptr->value.goal)
        {
            const_cast<ft_achievement *>(this)->set_error(ER_SUCCESS);
            game_achievement_restore_errno(guard, entry_errno);
            return (false);
        }
        ++goal_ptr;
    }
    const_cast<ft_achievement *>(this)->set_error(ER_SUCCESS);
    game_achievement_restore_errno(guard, entry_errno);
    return (true);
}

int ft_achievement::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_achievement *>(this)->set_error(error_code);
    game_achievement_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_achievement::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_achievement *>(this)->set_error(guard.get_error());
        game_achievement_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_achievement *>(this)->set_error(error_code);
    game_achievement_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_achievement::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
