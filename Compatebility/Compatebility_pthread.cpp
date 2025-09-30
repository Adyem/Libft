#include "compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
int cmp_thread_equal(pthread_t thread1, pthread_t thread2)
{
    return (thread1 == thread2);
}

int cmp_thread_cancel(pthread_t thread)
{
    if (TerminateThread((HANDLE)thread, 0) == 0)
    {
        ft_errno = GetLastError() + ERRNO_OFFSET;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int cmp_thread_yield()
{
    SwitchToThread();
    ft_errno = ER_SUCCESS;
    return (0);
}

int cmp_thread_sleep(unsigned int milliseconds)
{
    Sleep(milliseconds);
    ft_errno = ER_SUCCESS;
    return (0);
}
#else
# include <sched.h>
# include <unistd.h>
int cmp_thread_equal(pthread_t thread1, pthread_t thread2)
{
    return (pthread_equal(thread1, thread2));
}

int cmp_thread_cancel(pthread_t thread)
{
    int return_value = pthread_cancel(thread);
    if (return_value != 0)
    {
        ft_errno = return_value + ERRNO_OFFSET;
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}

int cmp_thread_yield()
{
    if (sched_yield() != 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int cmp_thread_sleep(unsigned int milliseconds)
{
    if (usleep(milliseconds * 1000) == -1)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
#endif
