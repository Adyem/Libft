#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"


int pt_mutex::unlock(pthread_t thread_id)
{
    this->set_error(ER_SUCCESS);
    if (this->_owner.load(std::memory_order_relaxed) != thread_id)
    {
        ft_errno = PT_ERR_MUTEX_OWNER;
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (-1);
    }
    this->_owner.store(0, std::memory_order_relaxed);
    this->_lock = false;
    this->_serving.fetch_add(1, std::memory_order_release);
    if (pt_thread_wake_one_uint32(this->_serving.native_handle()) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    return (FT_SUCCESS);
}

