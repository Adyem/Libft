#include "game_quest.hpp"
#include <utility>
#include "../Template/move.hpp"

ft_quest::ft_quest() noexcept
    : _id(0), _phases(0), _current_phase(0), _description(), _objective(), _reward_experience(0), _reward_items(), _error(ER_SUCCESS)
{
    if (this->_description.get_error() != ER_SUCCESS)
        this->set_error(this->_description.get_error());
    if (this->_error == ER_SUCCESS && this->_objective.get_error() != ER_SUCCESS)
        this->set_error(this->_objective.get_error());
    if (this->_error == ER_SUCCESS && this->_reward_items.get_error() != ER_SUCCESS)
        this->set_error(this->_reward_items.get_error());
    if (this->_error == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    return ;
}

ft_quest::ft_quest(const ft_quest &other) noexcept
    : _id(other._id), _phases(other._phases), _current_phase(other._current_phase), _description(other._description), _objective(other._objective), _reward_experience(other._reward_experience), _reward_items(), _error(ER_SUCCESS)
{
    if (this->_description.get_error() != ER_SUCCESS)
        this->set_error(this->_description.get_error());
    if (this->_error == ER_SUCCESS && other._description.get_error() != ER_SUCCESS)
        this->set_error(other._description.get_error());
    if (this->_error == ER_SUCCESS && this->_objective.get_error() != ER_SUCCESS)
        this->set_error(this->_objective.get_error());
    if (this->_error == ER_SUCCESS && other._objective.get_error() != ER_SUCCESS)
        this->set_error(other._objective.get_error());
    if (this->_error == ER_SUCCESS && this->_reward_items.get_error() != ER_SUCCESS)
        this->set_error(this->_reward_items.get_error());
    if (this->_error != ER_SUCCESS)
        return ;
    this->set_reward_items(other._reward_items);
    if (this->_error == ER_SUCCESS && other._error != ER_SUCCESS)
    {
        this->set_error(other._error);
        return ;
    }
    if (this->_error == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    return ;
}

ft_quest &ft_quest::operator=(const ft_quest &other) noexcept
{
    if (this != &other)
    {
        int other_error = other._error;
        this->_id = other._id;
        this->_phases = other._phases;
        this->_current_phase = other._current_phase;
        this->_description = other._description;
        if (this->_description.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_description.get_error());
            return (*this);
        }
        if (other._description.get_error() != ER_SUCCESS)
        {
            this->set_error(other._description.get_error());
            return (*this);
        }
        this->_objective = other._objective;
        if (this->_objective.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_objective.get_error());
            return (*this);
        }
        if (other._objective.get_error() != ER_SUCCESS)
        {
            this->set_error(other._objective.get_error());
            return (*this);
        }
        this->_reward_experience = other._reward_experience;
        this->set_reward_items(other._reward_items);
        if (this->_error != ER_SUCCESS)
            return (*this);
        if (other_error != ER_SUCCESS)
        {
            this->set_error(other_error);
            return (*this);
        }
        this->set_error(ER_SUCCESS);
    }
    return (*this);
}

ft_quest::ft_quest(ft_quest &&other) noexcept
    : _id(other._id), _phases(other._phases), _current_phase(other._current_phase), _description(ft_move(other._description)), _objective(ft_move(other._objective)), _reward_experience(other._reward_experience), _reward_items(ft_move(other._reward_items)), _error(other._error)
{
    if (this->_description.get_error() != ER_SUCCESS)
        this->set_error(this->_description.get_error());
    if (this->_error == ER_SUCCESS && this->_objective.get_error() != ER_SUCCESS)
        this->set_error(this->_objective.get_error());
    if (this->_error == ER_SUCCESS && this->_reward_items.get_error() != ER_SUCCESS)
        this->set_error(this->_reward_items.get_error());
    other._id = 0;
    other._phases = 0;
    other._current_phase = 0;
    other._reward_experience = 0;
    other.set_error(ER_SUCCESS);
    if (this->_error == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(this->_error);
    return ;
}

ft_quest &ft_quest::operator=(ft_quest &&other) noexcept
{
    if (this != &other)
    {
        int other_error = other._error;
        this->_id = other._id;
        this->_phases = other._phases;
        this->_current_phase = other._current_phase;
        this->_description = ft_move(other._description);
        if (this->_description.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_description.get_error());
            return (*this);
        }
        this->_objective = ft_move(other._objective);
        if (this->_objective.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_objective.get_error());
            return (*this);
        }
        this->_reward_experience = other._reward_experience;
        this->_reward_items = ft_move(other._reward_items);
        if (this->_reward_items.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_reward_items.get_error());
            return (*this);
        }
        this->set_error(other_error);
        other._id = 0;
        other._phases = 0;
        other._current_phase = 0;
        other._reward_experience = 0;
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

int ft_quest::get_id() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_id);
}

void ft_quest::set_id(int id) noexcept
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

int ft_quest::get_phases() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_phases);
}

void ft_quest::set_phases(int phases) noexcept
{
    if (phases < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_phases = phases;
    if (this->_current_phase > this->_phases)
        this->_current_phase = this->_phases;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_quest::get_current_phase() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_current_phase);
}

void ft_quest::set_current_phase(int phase) noexcept
{
    if (phase < 0 || phase > this->_phases)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_current_phase = phase;
    this->set_error(ER_SUCCESS);
    return ;
}

const ft_string &ft_quest::get_description() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_description);
}

void ft_quest::set_description(const ft_string &description) noexcept
{
    if (description.get_error() != ER_SUCCESS)
    {
        this->set_error(description.get_error());
        return ;
    }
    this->_description = description;
    if (this->_description.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_description.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

const ft_string &ft_quest::get_objective() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_objective);
}

void ft_quest::set_objective(const ft_string &objective) noexcept
{
    if (objective.get_error() != ER_SUCCESS)
    {
        this->set_error(objective.get_error());
        return ;
    }
    this->_objective = objective;
    if (this->_objective.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_objective.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_quest::get_reward_experience() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_reward_experience);
}

void ft_quest::set_reward_experience(int experience) noexcept
{
    if (experience < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_reward_experience = experience;
    this->set_error(ER_SUCCESS);
    return ;
}

ft_vector<ft_sharedptr<ft_item> > &ft_quest::get_reward_items() noexcept
{
    if (this->_reward_items.get_error() != ER_SUCCESS)
        this->set_error(this->_reward_items.get_error());
    else
        this->set_error(ER_SUCCESS);
    return (this->_reward_items);
}

const ft_vector<ft_sharedptr<ft_item> > &ft_quest::get_reward_items() const noexcept
{
    if (this->_reward_items.get_error() != ER_SUCCESS)
        this->set_error(this->_reward_items.get_error());
    else
        this->set_error(ER_SUCCESS);
    return (this->_reward_items);
}

void ft_quest::set_reward_items(const ft_vector<ft_sharedptr<ft_item> > &items) noexcept
{
    if (items.get_error() != ER_SUCCESS)
    {
        this->set_error(items.get_error());
        return ;
    }
    this->_reward_items.clear();
    if (this->_reward_items.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_reward_items.get_error());
        return ;
    }
    size_t index = 0;
    size_t count = items.size();
    while (index < count)
    {
        const ft_sharedptr<ft_item> &item_ptr = items[index];
        if (items.get_error() != ER_SUCCESS)
        {
            this->set_error(items.get_error());
            return ;
        }
        if (!item_ptr)
        {
            this->set_error(FT_ERR_INVALID_POINTER);
            return ;
        }
        if (item_ptr.get_error() != ER_SUCCESS)
        {
            this->set_error(item_ptr.get_error());
            return ;
        }
        if (item_ptr->get_error() != ER_SUCCESS)
        {
            this->set_error(item_ptr->get_error());
            return ;
        }
        this->_reward_items.push_back(item_ptr);
        if (this->_reward_items.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_reward_items.get_error());
            return ;
        }
        index++;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

bool ft_quest::is_complete() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_current_phase >= this->_phases);
}

void ft_quest::advance_phase() noexcept
{
    if (this->_phases <= 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (this->_current_phase >= this->_phases)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    ++this->_current_phase;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_quest::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_quest::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_quest::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
