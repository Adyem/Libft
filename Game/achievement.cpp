#include "achievement.hpp"

ft_achievement::ft_achievement() noexcept
    : _id(0), _goal(0), _progress(0)
{
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

int ft_achievement::get_goal() const noexcept
{
    return (this->_goal);
}

void ft_achievement::set_goal(int goal) noexcept
{
    this->_goal = goal;
    return ;
}

int ft_achievement::get_progress() const noexcept
{
    return (this->_progress);
}

void ft_achievement::set_progress(int progress) noexcept
{
    this->_progress = progress;
    return ;
}

void ft_achievement::add_progress(int value) noexcept
{
    this->_progress += value;
    return ;
}

bool ft_achievement::is_complete() const noexcept
{
    return (this->_progress >= this->_goal);
}

