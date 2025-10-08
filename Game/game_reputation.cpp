#include "game_reputation.hpp"
#include <utility>

ft_reputation::ft_reputation() noexcept
    : _milestones(), _reps(), _total_rep(0),
      _current_rep(0), _error(ER_SUCCESS)
{
    if (this->_milestones.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_milestones.get_error());
        return ;
    }
    if (this->_reps.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_reps.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

ft_reputation::ft_reputation(const ft_map<int, int> &milestones, int total) noexcept
    : _milestones(milestones), _reps(), _total_rep(total),
      _current_rep(0), _error(ER_SUCCESS)
{
    if (this->_milestones.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_milestones.get_error());
        return ;
    }
    if (this->_reps.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_reps.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

ft_reputation::ft_reputation(const ft_reputation &other) noexcept
    : _milestones(other._milestones), _reps(other._reps), _total_rep(other._total_rep),
      _current_rep(other._current_rep), _error(other._error)
{
    if (this->_milestones.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_milestones.get_error());
        return ;
    }
    if (this->_reps.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_reps.get_error());
        return ;
    }
    this->set_error(other._error);
    return ;
}

ft_reputation &ft_reputation::operator=(const ft_reputation &other) noexcept
{
    if (this != &other)
    {
        this->_milestones = other._milestones;
        this->_reps = other._reps;
        this->_total_rep = other._total_rep;
        this->_current_rep = other._current_rep;
        if (this->_milestones.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_milestones.get_error());
            return (*this);
        }
        if (this->_reps.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_reps.get_error());
            return (*this);
        }
        this->set_error(other._error);
    }
    return (*this);
}

ft_reputation::ft_reputation(ft_reputation &&other) noexcept
    : _milestones(std::move(other._milestones)), _reps(std::move(other._reps)), _total_rep(other._total_rep),
      _current_rep(other._current_rep), _error(other._error)
{
    if (this->_milestones.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_milestones.get_error());
        return ;
    }
    if (this->_reps.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_reps.get_error());
        return ;
    }
    this->set_error(this->_error);
    other._total_rep = 0;
    other._current_rep = 0;
    other._error = ER_SUCCESS;
    other._milestones.clear();
    other._reps.clear();
    return ;
}

ft_reputation &ft_reputation::operator=(ft_reputation &&other) noexcept
{
    if (this != &other)
    {
        this->_milestones = std::move(other._milestones);
        this->_reps = std::move(other._reps);
        this->_total_rep = other._total_rep;
        this->_current_rep = other._current_rep;
        if (this->_milestones.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_milestones.get_error());
            return (*this);
        }
        if (this->_reps.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_reps.get_error());
            return (*this);
        }
        this->set_error(other._error);
        other._total_rep = 0;
        other._current_rep = 0;
        other._error = ER_SUCCESS;
        other._milestones.clear();
        other._reps.clear();
    }
    return (*this);
}

int ft_reputation::get_total_rep() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_total_rep);
}

void ft_reputation::set_total_rep(int rep) noexcept
{
    this->_total_rep = rep;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_reputation::add_total_rep(int rep) noexcept
{
    this->_total_rep += rep;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_reputation::sub_total_rep(int rep) noexcept
{
    this->_total_rep -= rep;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_reputation::get_current_rep() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_current_rep);
}

void ft_reputation::set_current_rep(int rep) noexcept
{
    this->_current_rep = rep;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_reputation::add_current_rep(int rep) noexcept
{
    this->_current_rep += rep;
    this->_total_rep += rep;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_reputation::sub_current_rep(int rep) noexcept
{
    this->_current_rep -= rep;
    this->_total_rep -= rep;
    this->set_error(ER_SUCCESS);
    return ;
}

ft_map<int, int> &ft_reputation::get_milestones() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_milestones);
}

const ft_map<int, int> &ft_reputation::get_milestones() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_milestones);
}

void ft_reputation::set_milestones(const ft_map<int, int> &milestones) noexcept
{
    this->_milestones = milestones;
    if (this->_milestones.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_milestones.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_reputation::get_milestone(int id) const noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const Pair<int, int> *entry = this->_milestones.find(id);
    if (!entry)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (entry->value);
}

void ft_reputation::set_milestone(int id, int value) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    Pair<int, int> *entry = this->_milestones.find(id);
    if (!entry)
    {
        this->_milestones.insert(id, value);
        if (this->_milestones.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_milestones.get_error());
            return ;
        }
    }
    else
        entry->value = value;
    this->set_error(ER_SUCCESS);
    return ;
}

ft_map<int, int> &ft_reputation::get_reps() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_reps);
}

const ft_map<int, int> &ft_reputation::get_reps() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_reps);
}

void ft_reputation::set_reps(const ft_map<int, int> &reps) noexcept
{
    this->_reps = reps;
    if (this->_reps.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_reps.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_reputation::get_rep(int id) const noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const Pair<int, int> *entry = this->_reps.find(id);
    if (!entry)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (entry->value);
}

void ft_reputation::set_rep(int id, int value) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    Pair<int, int> *entry = this->_reps.find(id);
    if (!entry)
    {
        this->_reps.insert(id, value);
        if (this->_reps.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_reps.get_error());
            return ;
        }
    }
    else
        entry->value = value;
    this->set_error(ER_SUCCESS);
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
