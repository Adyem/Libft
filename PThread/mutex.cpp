#include "PThread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"

pt_mutex::pt_mutex()
{
    this->_flag.clear();
    this->_lock = false;
    this->_error = ER_SUCCESS;
}

pt_mutex::~pt_mutex()
{
    return ;
}

void    pt_mutex::set_error(int error)
{
    this->_error = error;
    ft_errno = error;
}

const volatile bool &pt_mutex::lockState() const
{
    return (this->_lock);
}
