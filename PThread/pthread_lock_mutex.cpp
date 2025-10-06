#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::lock(pthread_t thread_id)
{
    if (this->_owner.load(std::memory_order_relaxed) == thread_id)
    {
        ft_errno = PT_ERR_ALREADY_LOCKED;
        this->set_error(PT_ERR_ALREADY_LOCKED);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    const uint32_t my = this->_next.fetch_add(1, std::memory_order_acq_rel);
    while (1)
    {
        uint32_t cur = this->_serving.load(std::memory_order_acquire);
        if (cur == my)
            break;
        if (pt_thread_wait_uint32(&this->_serving, cur) != 0)
        {
            this->set_error(ft_errno);
            return (-1);
        }
    }
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock = true;
    return (FT_SUCCESS);
}
