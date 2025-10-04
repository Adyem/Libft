#include "compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <synchapi.h>
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

int cmp_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
    BOOL wait_result;
    DWORD error_code;

    while (1)
    {
        wait_result = WaitOnAddress(reinterpret_cast<volatile VOID *>(address),
                &expected_value, sizeof(uint32_t), INFINITE);
        if (wait_result != FALSE)
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        error_code = GetLastError();
        if (error_code == ERROR_SUCCESS)
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        if (error_code == ERROR_TIMEOUT)
            continue;
        ft_errno = error_code + ERRNO_OFFSET;
        return (-1);
    }
}

int cmp_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
    WakeByAddressSingle(reinterpret_cast<volatile VOID *>(address));
    ft_errno = ER_SUCCESS;
    return (0);
}
#else
# include <sched.h>
# include <unistd.h>
# include <linux/futex.h>
# include <sys/syscall.h>
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

int cmp_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
    int syscall_result;

    while (1)
    {
        syscall_result = syscall(SYS_futex, reinterpret_cast<uint32_t *>(address),
# ifdef FUTEX_WAIT_PRIVATE
                FUTEX_WAIT_PRIVATE,
# else
                FUTEX_WAIT,
# endif
                expected_value, NULL, NULL, 0);
        if (syscall_result == 0)
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        if (errno == EAGAIN)
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        if (errno == EINTR)
            continue;
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
}

int cmp_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
    int syscall_result;

    syscall_result = syscall(SYS_futex, reinterpret_cast<uint32_t *>(address),
# ifdef FUTEX_WAKE_PRIVATE
            FUTEX_WAKE_PRIVATE,
# else
            FUTEX_WAKE,
# endif
            1, NULL, NULL, 0);
    if (syscall_result == -1)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
#endif
