#include "PThread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"

pt_mutex::pt_mutex()
    : _next(0), _serving(0), _owner(0), _lock(false), _error(ER_SUCCESS)
{
    return ;
}

pt_mutex::~pt_mutex()
{
    return ;
}

void    pt_mutex::set_error(int error)
{
    _error = error;
    ft_errno = error;
    return ;
}

const volatile bool &pt_mutex::lockState() const
{
    return (_lock);
}

