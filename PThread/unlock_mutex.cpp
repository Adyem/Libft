#include "mutex.hpp"
#include "../Errno/errno.hpp"

thread_local int ft_errno = 0;

int pt_mutex::unlock(pthread_t thread_id)
{
    this->set_error(ER_SUCCESS);
    if (this->_thread_id != thread_id)
    {
		ft_errno = PT_ERR_MUTEX_OWNER;
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (-1);
    }
    this->_thread_id = 0;
    this->_lock = false;
    this->_lock_released = true;
    return (0); 
}
