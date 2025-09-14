#include "game_quest.hpp"

ft_quest::ft_quest() noexcept
    : _id(0), _phases(0), _current_phase(0), _description(), _objective(), _reward_experience(0), _reward_items()
{
    return ;
}

ft_quest::ft_quest(const ft_quest &other) noexcept
    : _id(other._id), _phases(other._phases), _current_phase(other._current_phase), _description(other._description), _objective(other._objective), _reward_experience(other._reward_experience), _reward_items()
{
    this->set_reward_items(other._reward_items);
    return ;
}

ft_quest &ft_quest::operator=(const ft_quest &other) noexcept
{
    if (this != &other)
    {
        this->_id = other._id;
        this->_phases = other._phases;
        this->_current_phase = other._current_phase;
        this->_description = other._description;
        this->_objective = other._objective;
        this->_reward_experience = other._reward_experience;
        this->set_reward_items(other._reward_items);
    }
    return (*this);
}

ft_quest::ft_quest(ft_quest &&other) noexcept
    : _id(other._id), _phases(other._phases), _current_phase(other._current_phase), _description(ft_move(other._description)), _objective(ft_move(other._objective)), _reward_experience(other._reward_experience), _reward_items(ft_move(other._reward_items))
{
    other._id = 0;
    other._phases = 0;
    other._current_phase = 0;
    other._reward_experience = 0;
    return ;
}

ft_quest &ft_quest::operator=(ft_quest &&other) noexcept
{
    if (this != &other)
    {
        this->_id = other._id;
        this->_phases = other._phases;
        this->_current_phase = other._current_phase;
        this->_description = ft_move(other._description);
        this->_objective = ft_move(other._objective);
        this->_reward_experience = other._reward_experience;
        this->_reward_items = ft_move(other._reward_items);
        other._id = 0;
        other._phases = 0;
        other._current_phase = 0;
        other._reward_experience = 0;
    }
    return (*this);
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

const ft_string &ft_quest::get_description() const noexcept
{
    return (this->_description);
}

void ft_quest::set_description(const ft_string &description) noexcept
{
    this->_description = description;
    return ;
}

const ft_string &ft_quest::get_objective() const noexcept
{
    return (this->_objective);
}

void ft_quest::set_objective(const ft_string &objective) noexcept
{
    this->_objective = objective;
    return ;
}

int ft_quest::get_reward_experience() const noexcept
{
    return (this->_reward_experience);
}

void ft_quest::set_reward_experience(int experience) noexcept
{
    this->_reward_experience = experience;
    return ;
}

ft_vector<ft_sharedptr<ft_item> > &ft_quest::get_reward_items() noexcept
{
    return (this->_reward_items);
}

const ft_vector<ft_sharedptr<ft_item> > &ft_quest::get_reward_items() const noexcept
{
    return (this->_reward_items);
}

void ft_quest::set_reward_items(const ft_vector<ft_sharedptr<ft_item> > &items) noexcept
{
    this->_reward_items.clear();
    size_t index = 0;
    size_t count = items.size();
    while (index < count)
    {
        this->_reward_items.push_back(items[index]);
        index++;
    }
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
