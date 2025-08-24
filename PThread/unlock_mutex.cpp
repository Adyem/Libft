#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

thread_local int ft_errno = 0;

int pt_mutex::unlock(pthread_t thread_id)
{
    set_error(ER_SUCCESS);
    if (_owner.load(std::memory_order_relaxed) != thread_id)
    {
        ft_errno = PT_ERR_MUTEX_OWNER;
        set_error(PT_ERR_MUTEX_OWNER);
        return (-1);
    }
    _owner.store(0, std::memory_order_relaxed);
    _lock = false;
    _serving.fetch_add(1, std::memory_order_release);
    return (SUCCES);
}

