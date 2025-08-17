#include <cerrno>
#include <pthread.h>
#include "PThread.hpp"
#include "../Errno/errno.hpp"

int pt_thread_detach(pthread_t thread)
{
        int return_value = pthread_detach(thread);
        if (return_value != 0)
                ft_errno = errno + ERRNO_OFFSET;
        return (return_value);
}
