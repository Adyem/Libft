#include "game_progress_tracker.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include "../Template/pair.hpp"
#include "../PThread/pthread.hpp"

static void game_progress_tracker_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int ft_progress_tracker::lock_pair(const ft_progress_tracker &first, const ft_progress_tracker &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_progress_tracker *ordered_first;
    const ft_progress_tracker *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_progress_tracker *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
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
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_progress_tracker_sleep_backoff();
    }
}

ft_progress_tracker::ft_progress_tracker() noexcept
    : _achievements(), _quests(), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    if (this->_achievements.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_achievements.get_error());
        return ;
    }
    if (this->_quests.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_quests.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_progress_tracker::~ft_progress_tracker() noexcept
{
    return ;
}

ft_progress_tracker::ft_progress_tracker(const ft_progress_tracker &other) noexcept
    : _achievements(), _quests(), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_achievements = other._achievements;
    if (this->_achievements.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_achievements.get_error());
        return ;
    }
    this->_quests = other._quests;
    if (this->_quests.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_quests.get_error());
        return ;
    }
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    return ;
}

ft_progress_tracker &ft_progress_tracker::operator=(const ft_progress_tracker &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_progress_tracker::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_achievements = other._achievements;
    if (this->_achievements.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_achievements.get_error());
        return (*this);
    }
    this->_quests = other._quests;
    if (this->_quests.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_quests.get_error());
        return (*this);
    }
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    return (*this);
}

ft_progress_tracker::ft_progress_tracker(ft_progress_tracker &&other) noexcept
    : _achievements(), _quests(), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_achievements = ft_move(other._achievements);
    if (this->_achievements.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_achievements.get_error());
        return ;
    }
    this->_quests = ft_move(other._quests);
    if (this->_quests.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_quests.get_error());
        return ;
    }
    this->_error_code = other._error_code;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_progress_tracker &ft_progress_tracker::operator=(ft_progress_tracker &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_progress_tracker::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_achievements = ft_move(other._achievements);
    if (this->_achievements.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_achievements.get_error());
        return (*this);
    }
    this->_quests = ft_move(other._quests);
    if (this->_quests.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_quests.get_error());
        return (*this);
    }
    this->_error_code = other._error_code;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    return (*this);
}

ft_map<int, ft_achievement> &ft_progress_tracker::get_achievements() noexcept
{
    return (this->_achievements);
}

const ft_map<int, ft_achievement> &ft_progress_tracker::get_achievements() const noexcept
{
    return (this->_achievements);
}

ft_map<int, ft_quest> &ft_progress_tracker::get_quests() noexcept
{
    return (this->_quests);
}

const ft_map<int, ft_quest> &ft_progress_tracker::get_quests() const noexcept
{
    return (this->_quests);
}

void ft_progress_tracker::set_achievements(const ft_map<int, ft_achievement> &achievements) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_achievements = achievements;
    if (this->_achievements.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_achievements.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_progress_tracker::set_quests(const ft_map<int, ft_quest> &quests) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_quests = quests;
    if (this->_quests.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_quests.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_progress_tracker::register_achievement(const ft_achievement &achievement) noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    identifier = achievement.get_id();
    this->_achievements.insert(identifier, achievement);
    if (this->_achievements.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_achievements.get_error());
        return (this->_achievements.get_error());
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int ft_progress_tracker::register_quest(const ft_quest &quest) noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    identifier = quest.get_id();
    this->_quests.insert(identifier, quest);
    if (this->_quests.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_quests.get_error());
        return (this->_quests.get_error());
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int ft_progress_tracker::update_goal_target(int achievement_id, int goal_id, int target) noexcept
{
    Pair<int, ft_achievement> *achievement_pair;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    achievement_pair = this->_achievements.find(achievement_id);
    if (achievement_pair == this->_achievements.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    achievement_pair->value.set_goal(goal_id, target);
    this->set_error(achievement_pair->value.get_error());
    return (this->_error_code);
}

int ft_progress_tracker::update_goal_progress(int achievement_id, int goal_id, int progress) noexcept
{
    Pair<int, ft_achievement> *achievement_pair;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    achievement_pair = this->_achievements.find(achievement_id);
    if (achievement_pair == this->_achievements.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    achievement_pair->value.set_progress(goal_id, progress);
    this->set_error(achievement_pair->value.get_error());
    return (this->_error_code);
}

int ft_progress_tracker::add_goal_progress(int achievement_id, int goal_id, int value) noexcept
{
    Pair<int, ft_achievement> *achievement_pair;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    achievement_pair = this->_achievements.find(achievement_id);
    if (achievement_pair == this->_achievements.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    achievement_pair->value.add_progress(goal_id, value);
    this->set_error(achievement_pair->value.get_error());
    return (this->_error_code);
}

bool ft_progress_tracker::is_achievement_complete(int achievement_id) const noexcept
{
    const Pair<int, ft_achievement> *achievement_pair;
    bool completed;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_progress_tracker *>(this)->set_error(guard.get_error());
        return (false);
    }
    achievement_pair = this->_achievements.find(achievement_id);
    if (achievement_pair == this->_achievements.end())
    {
        const_cast<ft_progress_tracker *>(this)->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    completed = achievement_pair->value.is_complete();
    const_cast<ft_progress_tracker *>(this)->set_error(achievement_pair->value.get_error());
    return (completed);
}

int ft_progress_tracker::set_quest_phase(int quest_id, int phase) noexcept
{
    Pair<int, ft_quest> *quest_pair;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    quest_pair = this->_quests.find(quest_id);
    if (quest_pair == this->_quests.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    quest_pair->value.set_current_phase(phase);
    this->set_error(quest_pair->value.get_error());
    return (this->_error_code);
}

int ft_progress_tracker::advance_quest_phase(int quest_id) noexcept
{
    Pair<int, ft_quest> *quest_pair;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    quest_pair = this->_quests.find(quest_id);
    if (quest_pair == this->_quests.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    quest_pair->value.advance_phase();
    this->set_error(quest_pair->value.get_error());
    return (this->_error_code);
}

bool ft_progress_tracker::is_quest_complete(int quest_id) const noexcept
{
    const Pair<int, ft_quest> *quest_pair;
    bool complete;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_progress_tracker *>(this)->set_error(guard.get_error());
        return (false);
    }
    quest_pair = this->_quests.find(quest_id);
    if (quest_pair == this->_quests.end())
    {
        const_cast<ft_progress_tracker *>(this)->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    complete = quest_pair->value.is_complete();
    const_cast<ft_progress_tracker *>(this)->set_error(quest_pair->value.get_error());
    return (complete);
}

int ft_progress_tracker::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_progress_tracker *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_progress_tracker *>(this)->set_error(FT_ERR_SUCCESSS);
    return (error_code);
}

const char *ft_progress_tracker::get_error_str() const noexcept
{
    const char *error_string;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_progress_tracker *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_string = ft_strerror(this->_error_code);
    const_cast<ft_progress_tracker *>(this)->set_error(FT_ERR_SUCCESSS);
    return (error_string);
}

void ft_progress_tracker::set_error(int error) const noexcept
{
    const_cast<ft_progress_tracker *>(this)->_error_code = error;
    ft_errno = error;
    return ;
}
