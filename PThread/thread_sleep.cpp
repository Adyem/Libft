#include "PThread.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>

#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

int pt_thread_sleep(unsigned int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
    return 0;
#else
    if (usleep(milliseconds * 1000) == -1)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return -1;
    }
    return 0;
#endif
}
