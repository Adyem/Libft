#include "achievement.hpp"

ft_achievement::ft_achievement() noexcept
    : _id(0), _goals(), _error(ER_SUCCESS)
{
    if (this->_goals.get_error() != ER_SUCCESS)
        this->set_error(this->_goals.get_error());
    return ;
}

int ft_achievement::get_id() const noexcept
{
    return (this->_id);
}

void ft_achievement::set_id(int id) noexcept
{
    this->_id = id;
    return ;
}

ft_map<int, ft_goal> &ft_achievement::get_goals() noexcept
{
    return (this->_goals);
}

const ft_map<int, ft_goal> &ft_achievement::get_goals() const noexcept
{
    return (this->_goals);
}

void ft_achievement::set_goals(const ft_map<int, ft_goal> &goals) noexcept
{
    this->_goals = goals;
    return ;
}

int ft_achievement::get_goal(int id) const noexcept
{
    const Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
        return (0);
    return (entry->value.goal);
}

void ft_achievement::set_goal(int id, int goal) noexcept
{
    Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal{goal, 0};
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
            this->set_error(this->_goals.get_error());
    }
    else
        entry->value.goal = goal;
    return ;
}

int ft_achievement::get_progress(int id) const noexcept
{
    const Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
        return (0);
    return (entry->value.progress);
}

void ft_achievement::set_progress(int id, int progress) noexcept
{
    Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal{0, progress};
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
            this->set_error(this->_goals.get_error());
    }
    else
        entry->value.progress = progress;
    return ;
}

void ft_achievement::add_progress(int id, int value) noexcept
{
    Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal{0, value};
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
            this->set_error(this->_goals.get_error());
    }
    else
        entry->value.progress += value;
    return ;
}

bool ft_achievement::is_goal_complete(int id) const noexcept
{
    const Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
        return (false);
    return (entry->value.progress >= entry->value.goal);
}

bool ft_achievement::is_complete() const noexcept
{
    const Pair<int, ft_goal> *ptr = this->_goals.end() - this->_goals.getSize();
    const Pair<int, ft_goal> *end = this->_goals.end();
    while (ptr != end)
    {
        if (ptr->value.progress < ptr->value.goal)
            return (false);
        ++ptr;
    }
    return (true);
}

int ft_achievement::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_achievement::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_achievement::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

