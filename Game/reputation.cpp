#include "reputation.hpp"

ft_reputation::ft_reputation() noexcept
    : _milestones(), _reps(), _total_rep(0),
      _current_rep(0), _error(ER_SUCCESS)
{
    if (this->_milestones.get_error() != ER_SUCCESS)
        this->set_error(this->_milestones.get_error());
    else if (this->_reps.get_error() != ER_SUCCESS)
        this->set_error(this->_reps.get_error());
    return ;
}

ft_reputation::ft_reputation(const ft_map<int, int> &milestones, int total) noexcept
    : _milestones(milestones), _reps(), _total_rep(total),
      _current_rep(0), _error(ER_SUCCESS)
{
    if (this->_milestones.get_error() != ER_SUCCESS)
        this->set_error(this->_milestones.get_error());
    else if (this->_reps.get_error() != ER_SUCCESS)
        this->set_error(this->_reps.get_error());
    return ;
}

int ft_reputation::get_total_rep() const noexcept
{
    return (this->_total_rep);
}

void ft_reputation::set_total_rep(int rep) noexcept
{
    this->_total_rep = rep;
    return ;
}

void ft_reputation::add_total_rep(int rep) noexcept
{
    this->_total_rep += rep;
    return ;
}

void ft_reputation::sub_total_rep(int rep) noexcept
{
    this->_total_rep -= rep;
    return ;
}

int ft_reputation::get_current_rep() const noexcept
{
    return (this->_current_rep);
}

void ft_reputation::set_current_rep(int rep) noexcept
{
    this->_current_rep = rep;
    return ;
}

void ft_reputation::add_current_rep(int rep) noexcept
{
    this->_current_rep += rep;
    this->_total_rep += rep;
    return ;
}

void ft_reputation::sub_current_rep(int rep) noexcept
{
    this->_current_rep -= rep;
    this->_total_rep -= rep;
    return ;
}

ft_map<int, int> &ft_reputation::get_milestones() noexcept
{
    return (this->_milestones);
}

const ft_map<int, int> &ft_reputation::get_milestones() const noexcept
{
    return (this->_milestones);
}

void ft_reputation::set_milestones(const ft_map<int, int> &milestones) noexcept
{
    this->_milestones = milestones;
    return ;
}

int ft_reputation::get_milestone(int id) const noexcept
{
    const Pair<int, int> *entry = this->_milestones.find(id);
    if (!entry)
        return (0);
    return (entry->value);
}

void ft_reputation::set_milestone(int id, int value) noexcept
{
    Pair<int, int> *entry = this->_milestones.find(id);
    if (!entry)
        this->_milestones.insert(id, value);
    else
        entry->value = value;
    return ;
}

ft_map<int, int> &ft_reputation::get_reps() noexcept
{
    return (this->_reps);
}

const ft_map<int, int> &ft_reputation::get_reps() const noexcept
{
    return (this->_reps);
}

void ft_reputation::set_reps(const ft_map<int, int> &reps) noexcept
{
    this->_reps = reps;
    return ;
}

int ft_reputation::get_rep(int id) const noexcept
{
    const Pair<int, int> *entry = this->_reps.find(id);
    if (!entry)
        return (0);
    return (entry->value);
}

void ft_reputation::set_rep(int id, int value) noexcept
{
    Pair<int, int> *entry = this->_reps.find(id);
    if (!entry)
        this->_reps.insert(id, value);
    else
        entry->value = value;
    return ;
}

int ft_reputation::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_reputation::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_reputation::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
