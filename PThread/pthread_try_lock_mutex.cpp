#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::try_lock(pthread_t thread_id) const
{
    this->set_error(ER_SUCCESS);
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock = true;
    return (FT_SUCCESS);
}

