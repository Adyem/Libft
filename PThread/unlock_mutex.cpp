#include "mutex.hpp"
#include "../Errno/errno.hpp"

thread_local int ft_errno = 0;

int pt_mutex::unlock(pthread_t thread_id)
{
    (void)thread_id;
    this->set_error(ER_SUCCESS);
    this->_flag.clear(std::memory_order_release);
    this->_lock = false;
    return (0);
}
