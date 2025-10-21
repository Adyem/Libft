#include "game_achievement.hpp"
#include <utility>
#include "../Template/move.hpp"

ft_achievement::ft_achievement() noexcept
    : _id(0), _goals(), _error(ER_SUCCESS)
{
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

ft_achievement::ft_achievement(const ft_achievement &other) noexcept
    : _id(other._id), _goals(other._goals), _error(other._error)
{
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        return ;
    }
    this->set_error(other._error);
    return ;
}

ft_achievement &ft_achievement::operator=(const ft_achievement &other) noexcept
{
    if (this != &other)
    {
        int other_error = other._error;
        this->_id = other._id;
        this->_goals = other._goals;
        if (this->_goals.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            return (*this);
        }
        this->set_error(other_error);
    }
    return (*this);
}

ft_achievement::ft_achievement(ft_achievement &&other) noexcept
    : _id(other._id), _goals(ft_move(other._goals)), _error(other._error)
{
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        return ;
    }
    this->set_error(this->_error);
    other._id = 0;
    other._error = ER_SUCCESS;
    other._goals.clear();
    return ;
}

ft_achievement &ft_achievement::operator=(ft_achievement &&other) noexcept
{
    if (this != &other)
    {
        int other_error = other._error;
        this->_id = other._id;
        this->_goals = ft_move(other._goals);
        if (this->_goals.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            return (*this);
        }
        this->set_error(other_error);
        other._id = 0;
        other._error = ER_SUCCESS;
        other._goals.clear();
    }
    return (*this);
}

int ft_achievement::get_id() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_id);
}

void ft_achievement::set_id(int id) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_id = id;
    this->set_error(ER_SUCCESS);
    return ;
}

ft_map<int, ft_goal> &ft_achievement::get_goals() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_goals);
}

const ft_map<int, ft_goal> &ft_achievement::get_goals() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_goals);
}

void ft_achievement::set_goals(const ft_map<int, ft_goal> &goals) noexcept
{
    this->_goals = goals;
    if (this->_goals.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_goals.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_achievement::get_goal(int id) const noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (entry->value.goal);
}

void ft_achievement::set_goal(int id, int goal) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal{goal, 0};
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            return ;
        }
    }
    else
        entry->value.goal = goal;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_achievement::get_progress(int id) const noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (entry->value.progress);
}

void ft_achievement::set_progress(int id, int progress) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal{0, progress};
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            return ;
        }
    }
    else
        entry->value.progress = progress;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_achievement::add_progress(int id, int value) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        ft_goal new_goal{0, value};
        this->_goals.insert(id, new_goal);
        if (this->_goals.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_goals.get_error());
            return ;
        }
    }
    else
        entry->value.progress += value;
    this->set_error(ER_SUCCESS);
    return ;
}

bool ft_achievement::is_goal_complete(int id) const noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    const Pair<int, ft_goal> *entry = this->_goals.find(id);
    if (!entry)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (entry->value.progress >= entry->value.goal);
}

bool ft_achievement::is_complete() const noexcept
{
    const Pair<int, ft_goal> *goal_ptr = this->_goals.end() - this->_goals.size();
    const Pair<int, ft_goal> *goal_end = this->_goals.end();
    while (goal_ptr != goal_end)
    {
        if (goal_ptr->value.progress < goal_ptr->value.goal)
        {
            this->set_error(ER_SUCCESS);
            return (false);
        }
        ++goal_ptr;
    }
    this->set_error(ER_SUCCESS);
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

