#include <cerrno>
#include "PThread.hpp"
#include "../Errno/errno.hpp"

int pt_thread_cancel(pthread_t thread)
{
#ifdef _WIN32
    if (TerminateThread((HANDLE)thread, 0) == 0)
    {
        ft_errno = GetLastError() + ERRNO_OFFSET;
        return -1;
    }
    return 0;
#else
    int return_value = pthread_cancel(thread);
    if (return_value != 0)
        ft_errno = errno + ERRNO_OFFSET;
    return return_value;
#endif
}

