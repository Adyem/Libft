#include "quest.hpp"

ft_quest::ft_quest() noexcept
    : _id(0), _phases(0), _current_phase(0)
{
    return ;
}

int ft_quest::get_id() const noexcept
{
    return (_id);
}

void ft_quest::set_id(int id) noexcept
{
    _id = id;
    return ;
}

int ft_quest::get_phases() const noexcept
{
    return (_phases);
}

void ft_quest::set_phases(int phases) noexcept
{
    _phases = phases;
    return ;
}

int ft_quest::get_current_phase() const noexcept
{
    return (_current_phase);
}

void ft_quest::set_current_phase(int phase) noexcept
{
    _current_phase = phase;
    return ;
}
