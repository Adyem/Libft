#include "PThread.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>

#ifdef _WIN32
# include <windows.h>
#else
# include <sched.h>
#endif

int pt_thread_yield()
{
#ifdef _WIN32
    SwitchToThread();
    return (0);
#else
    if (sched_yield() != 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    return (0);
#endif
}
