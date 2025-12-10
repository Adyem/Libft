#include "game_reputation.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_reputation_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_reputation_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_reputation::lock_pair(const ft_reputation &first, const ft_reputation &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_reputation *ordered_first;
    const ft_reputation *ordered_second;
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
        const ft_reputation *temporary;

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
        game_reputation_sleep_backoff();
    }
}

ft_reputation::ft_reputation() noexcept
    : _milestones(), _reps(), _total_rep(0),
      _current_rep(0), _error(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    if (this->_milestones.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_milestones.get_error());
        ft_errno = entry_errno;
        return ;
    }
    if (this->_reps.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_reps.get_error());
        ft_errno = entry_errno;
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    ft_errno = entry_errno;
    return ;
}

ft_reputation::ft_reputation(const ft_map<int, int> &milestones, int total) noexcept
    : _milestones(milestones), _reps(), _total_rep(total),
      _current_rep(0), _error(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    if (this->_milestones.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_milestones.get_error());
        ft_errno = entry_errno;
        return ;
    }
    if (this->_reps.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_reps.get_error());
        ft_errno = entry_errno;
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    ft_errno = entry_errno;
    return ;
}

ft_reputation::ft_reputation(const ft_reputation &other) noexcept
    : _milestones(), _reps(), _total_rep(0),
      _current_rep(0), _error(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->_milestones = ft_map<int, int>();
        this->_reps = ft_map<int, int>();
        this->_total_rep = 0;
        this->_current_rep = 0;
        this->set_error(other_guard.get_error());
        game_reputation_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_milestones = other._milestones;
    this->_reps = other._reps;
    this->_total_rep = other._total_rep;
    this->_current_rep = other._current_rep;
    this->_error = other._error;
    this->set_error(other._error);
    game_reputation_restore_errno(other_guard, entry_errno);
    return ;
}

ft_reputation &ft_reputation::operator=(const ft_reputation &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_reputation::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        game_reputation_restore_errno(this_guard, entry_errno);
        game_reputation_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_milestones = other._milestones;
    this->_reps = other._reps;
    this->_total_rep = other._total_rep;
    this->_current_rep = other._current_rep;
    this->_error = other._error;
    this->set_error(other._error);
    game_reputation_restore_errno(this_guard, entry_errno);
    game_reputation_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_reputation::ft_reputation(ft_reputation &&other) noexcept
    : _milestones(), _reps(), _total_rep(0),
      _current_rep(0), _error(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->_milestones = ft_map<int, int>();
        this->_reps = ft_map<int, int>();
        this->_total_rep = 0;
        this->_current_rep = 0;
        this->set_error(other_guard.get_error());
        game_reputation_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_milestones = ft_move(other._milestones);
    this->_reps = ft_move(other._reps);
    this->_total_rep = other._total_rep;
    this->_current_rep = other._current_rep;
    this->_error = other._error;
    other._total_rep = 0;
    other._current_rep = 0;
    other._error = FT_ER_SUCCESSS;
    other._milestones.clear();
    other._reps.clear();
    this->set_error(this->_error);
    other.set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(other_guard, entry_errno);
    return ;
}

ft_reputation &ft_reputation::operator=(ft_reputation &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_reputation::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        game_reputation_restore_errno(this_guard, entry_errno);
        game_reputation_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_milestones = ft_move(other._milestones);
    this->_reps = ft_move(other._reps);
    this->_total_rep = other._total_rep;
    this->_current_rep = other._current_rep;
    this->_error = other._error;
    other._total_rep = 0;
    other._current_rep = 0;
    other._error = FT_ER_SUCCESSS;
    other._milestones.clear();
    other._reps.clear();
    this->set_error(this->_error);
    other.set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(this_guard, entry_errno);
    game_reputation_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_reputation::get_total_rep() const noexcept
{
    int entry_errno;
    int total_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_reputation *>(this)->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (0);
    }
    total_value = this->_total_rep;
    const_cast<ft_reputation *>(this)->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return (total_value);
}

void ft_reputation::set_total_rep(int rep) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->_total_rep = rep;
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

void ft_reputation::add_total_rep(int rep) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->_total_rep += rep;
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

void ft_reputation::sub_total_rep(int rep) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->_total_rep -= rep;
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

int ft_reputation::get_current_rep() const noexcept
{
    int entry_errno;
    int current_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_reputation *>(this)->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (0);
    }
    current_value = this->_current_rep;
    const_cast<ft_reputation *>(this)->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return (current_value);
}

void ft_reputation::set_current_rep(int rep) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->_current_rep = rep;
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

void ft_reputation::add_current_rep(int rep) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->_current_rep += rep;
    this->_total_rep += rep;
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

void ft_reputation::sub_current_rep(int rep) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->_current_rep -= rep;
    this->_total_rep -= rep;
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

ft_map<int, int> &ft_reputation::get_milestones() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (this->_milestones);
    }
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return (this->_milestones);
}

const ft_map<int, int> &ft_reputation::get_milestones() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_reputation *>(this)->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (this->_milestones);
    }
    const_cast<ft_reputation *>(this)->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return (this->_milestones);
}

void ft_reputation::set_milestones(const ft_map<int, int> &milestones) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->_milestones = milestones;
    if (this->_milestones.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_milestones.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

int ft_reputation::get_milestone(int id) const noexcept
{
    int entry_errno;
    const Pair<int, int> *entry;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_reputation *>(this)->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (0);
    }
    if (id < 0)
    {
        const_cast<ft_reputation *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        game_reputation_restore_errno(guard, entry_errno);
        return (0);
    }
    entry = this->_milestones.find(id);
    if (entry == this->_milestones.end())
    {
        const_cast<ft_reputation *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_reputation_restore_errno(guard, entry_errno);
        return (0);
    }
    const_cast<ft_reputation *>(this)->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return (entry->value);
}

void ft_reputation::set_milestone(int id, int value) noexcept
{
    int entry_errno;
    Pair<int, int> *entry;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    entry = this->_milestones.find(id);
    if (entry == this->_milestones.end())
    {
        this->_milestones.insert(id, value);
        if (this->_milestones.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_milestones.get_error());
            game_reputation_restore_errno(guard, entry_errno);
            return ;
        }
    }
    else
        entry->value = value;
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

ft_map<int, int> &ft_reputation::get_reps() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (this->_reps);
    }
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return (this->_reps);
}

const ft_map<int, int> &ft_reputation::get_reps() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_reputation *>(this)->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (this->_reps);
    }
    const_cast<ft_reputation *>(this)->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return (this->_reps);
}

void ft_reputation::set_reps(const ft_map<int, int> &reps) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->_reps = reps;
    if (this->_reps.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_reps.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

int ft_reputation::get_rep(int id) const noexcept
{
    int entry_errno;
    const Pair<int, int> *entry;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_reputation *>(this)->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (0);
    }
    if (id < 0)
    {
        const_cast<ft_reputation *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        game_reputation_restore_errno(guard, entry_errno);
        return (0);
    }
    entry = this->_reps.find(id);
    if (entry == this->_reps.end())
    {
        const_cast<ft_reputation *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_reputation_restore_errno(guard, entry_errno);
        return (0);
    }
    const_cast<ft_reputation *>(this)->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return (entry->value);
}

void ft_reputation::set_rep(int id, int value) noexcept
{
    int entry_errno;
    Pair<int, int> *entry;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_reputation_restore_errno(guard, entry_errno);
        return ;
    }
    entry = this->_reps.find(id);
    if (entry == this->_reps.end())
    {
        this->_reps.insert(id, value);
        if (this->_reps.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_reps.get_error());
            game_reputation_restore_errno(guard, entry_errno);
            return ;
        }
    }
    else
        entry->value = value;
    this->set_error(FT_ER_SUCCESSS);
    game_reputation_restore_errno(guard, entry_errno);
    return ;
}

int ft_reputation::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_reputation *>(this)->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_reputation *>(this)->set_error(error_code);
    game_reputation_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_reputation::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_reputation *>(this)->set_error(guard.get_error());
        game_reputation_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_reputation *>(this)->set_error(error_code);
    game_reputation_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_reputation::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

