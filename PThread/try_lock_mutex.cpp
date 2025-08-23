#include "mutex.hpp"
#include "../Errno/errno.hpp"

#define FAILURE -1
#define SUCCES 0

int pt_mutex::try_lock(pthread_t thread_id)
{
    (void)thread_id;
    this->set_error(ER_SUCCESS);
    if (this->_flag.test_and_set(std::memory_order_acquire))
    {
        this->set_error(PT_ALREADDY_LOCKED);
        ft_errno = PT_ALREADDY_LOCKED;
        return (PT_ALREADDY_LOCKED);
    }
    this->_lock = true;
    return (SUCCES);
}
