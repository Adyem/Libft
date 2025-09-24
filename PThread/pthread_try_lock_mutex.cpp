#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::try_lock(pthread_t thread_id)
{
    this->set_error(ER_SUCCESS);
    if (this->_owner.load(std::memory_order_relaxed) == thread_id)
    {
        ft_errno = PT_ERR_ALRDY_LOCKED;
        this->set_error(PT_ERR_ALRDY_LOCKED);
        return (-1);
    }
    uint32_t cur_serving = this->_serving.load(std::memory_order_acquire);
    uint32_t cur_next = this->_next.load(std::memory_order_relaxed);
    if (cur_serving != cur_next)
    {
        ft_errno = PT_ALREADDY_LOCKED;
        this->set_error(PT_ALREADDY_LOCKED);
        return (PT_ALREADDY_LOCKED);
    }
    if (!this->_next.compare_exchange_strong(cur_next, cur_next + 1, std::memory_order_acq_rel))
    {
        ft_errno = PT_ALREADDY_LOCKED;
        this->set_error(PT_ALREADDY_LOCKED);
        return (PT_ALREADDY_LOCKED);
    }
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock = true;
    return (FT_SUCCESS);
}

