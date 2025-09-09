#include "world.hpp"

ft_world::ft_world() noexcept
    : _events(), _error(ER_SUCCESS)
{
    if (this->_events.get_error() != ER_SUCCESS)
        this->set_error(this->_events.get_error());
    return ;
}

ft_map<int, ft_event> &ft_world::get_events() noexcept
{
    return (this->_events);
}

const ft_map<int, ft_event> &ft_world::get_events() const noexcept
{
    return (this->_events);
}

int ft_world::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_world::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_world::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
