#include "PThread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <unistd.h>
#include <algorithm>

#undef FAILURE
#define FAILURE -1

int pt_mutex::lock(pthread_t thread_id)
{
    int			sleep_time = SLEEP_TIME;
    const int	max_sleep = MAX_SLEEP;

	if (this->_lock && this->_thread_id == thread_id)
	{
		ft_errno = PT_ERR_ALRDY_LOCKED;
		this->set_error(PT_ERR_ALRDY_LOCKED);
		return (FAILURE);
	}
	ft_errno = ER_SUCCESS;
	this->set_error(ER_SUCCESS);
    while (true)
    {
        if (this->_wait_queue_start == this->_wait_queue_end && !this->_lock)
        {
            if (__sync_bool_compare_and_swap(&this->_lock, false, true))
            {
                this->_thread_id = thread_id;
                return (SUCCES);
            }
        }
        bool already_waiting = false;
        int start = this->_wait_queue_start;
        int end = this->_wait_queue_end;
        while (start != end)
        {
            if (this->_wait_queue[start] == thread_id)
            {
                already_waiting = true;
                break ;
            }
            start = (start + 1) % 128;
        }
        if (!already_waiting)
        {
            int next_end = (this->_wait_queue_end + 1) % 128;
            if (next_end == this->_wait_queue_start)
            {
				ft_errno = PT_ERR_QUEUE_FULL;
                this->set_error(PT_ERR_QUEUE_FULL);
                return (FAILURE);
            }
            this->_wait_queue[this->_wait_queue_end] = thread_id;
            this->_wait_queue_end = next_end;
        }
        usleep(sleep_time);
        sleep_time = std::min(sleep_time * 2, max_sleep);
        if (this->_lock_released && this->_wait_queue[this->_wait_queue_start] == thread_id)
        {
            int next_start = (this->_wait_queue_start + 1) % 128;
            this->_wait_queue_start = next_start;
            if (__sync_bool_compare_and_swap(&this->_lock, false, true))
            {
                this->_thread_id = thread_id;
                return (SUCCES);
            }
        }
    }
}
