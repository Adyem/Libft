#include "quest.hpp"

ft_quest::ft_quest() noexcept
    : _id(0), _phases(0), _current_phase(0)
{
    return ;
}

int ft_quest::get_id() const noexcept
{
    return (this->_id);
}

void ft_quest::set_id(int id) noexcept
{
    this->_id = id;
    return ;
}

int ft_quest::get_phases() const noexcept
{
    return (this->_phases);
}

void ft_quest::set_phases(int phases) noexcept
{
    this->_phases = phases;
    return ;
}

int ft_quest::get_current_phase() const noexcept
{
    return (this->_current_phase);
}

void ft_quest::set_current_phase(int phase) noexcept
{
    this->_current_phase = phase;
    return ;
}

bool ft_quest::is_complete() const noexcept
{
    return (this->_current_phase >= this->_phases);
}

void ft_quest::advance_phase() noexcept
{
    if (this->_current_phase < this->_phases)
        ++this->_current_phase;
    return ;
}
