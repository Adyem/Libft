#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"

pt_mutex::pt_mutex()
    : _owner(0), _lock(false), _error(ER_SUCCESS)
{
    return ;
}

pt_mutex::~pt_mutex()
{
    return ;
}

void    pt_mutex::set_error(int error) const
{
    this->_error = error;
    ft_errno = error;
    return ;
}

const volatile bool &pt_mutex::lockState() const
{
    return (this->_lock);
}

int pt_mutex::get_error() const
{
    return (this->_error);
}

const char *pt_mutex::get_error_str() const
{
    return (ft_strerror(this->_error));
}

