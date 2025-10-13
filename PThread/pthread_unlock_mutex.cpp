#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::unlock(pthread_t thread_id) const
{
    (void)thread_id;
    this->set_error(ER_SUCCESS);
    this->_owner.store(0, std::memory_order_relaxed);
    this->_lock = false;
    return (FT_SUCCESS);
}

