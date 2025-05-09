#include "mutex.hpp"
#include "../Errno/errno.hpp"

#define FAILURE -1
#define SUCCES 0

int pt_mutex::try_lock(pthread_t thread_id)
{
    this->set_error(ER_SUCCESS);
	if (this->_lock && this->_thread_id == thread_id)
	{
		ft_errno = PT_ERR_ALRDY_LOCKED;
		this->set_error(PT_ERR_ALRDY_LOCKED);
		return (FAILURE);
	}
    if (__sync_bool_compare_and_swap(&this->_lock, false, true))
    {
		ft_errno = SUCCES;
        this->_thread_id = thread_id;
        return (SUCCES);
    }
    return (PT_ALREADDY_LOCKED);
}
