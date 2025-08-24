#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::try_lock(pthread_t thread_id)
{
    set_error(ER_SUCCESS);
    if (_owner.load(std::memory_order_relaxed) == thread_id)
    {
        ft_errno = PT_ERR_ALRDY_LOCKED;
        set_error(PT_ERR_ALRDY_LOCKED);
        return (-1);
    }
    uint32_t cur_serving = _serving.load(std::memory_order_acquire);
    uint32_t cur_next = _next.load(std::memory_order_relaxed);
    if (cur_serving != cur_next)
    {
        ft_errno = PT_ALREADDY_LOCKED;
        set_error(PT_ALREADDY_LOCKED);
        return (PT_ALREADDY_LOCKED);
    }
    if (!_next.compare_exchange_strong(cur_next, cur_next + 1, std::memory_order_acq_rel))
    {
        ft_errno = PT_ALREADDY_LOCKED;
        set_error(PT_ALREADDY_LOCKED);
        return (PT_ALREADDY_LOCKED);
    }
    _owner.store(thread_id, std::memory_order_relaxed);
    _lock = true;
    return (SUCCES);
}

