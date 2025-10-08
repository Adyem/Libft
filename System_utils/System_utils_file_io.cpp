#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>
#include <ctime>
#include <cstddef>
#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#endif

static t_su_write_syscall_hook    g_su_write_syscall_hook = ft_nullptr;

static ssize_t    su_default_write_syscall(int file_descriptor, const void *buffer, size_t count)
{
#if defined(_WIN32) || defined(_WIN64)
    return (cmp_write(file_descriptor, buffer, static_cast<unsigned int>(count)));
#else
    return (cmp_write(file_descriptor, buffer, count));
#endif
}

void    su_set_write_syscall_hook(t_su_write_syscall_hook hook)
{
    if (hook != ft_nullptr)
    {
        g_su_write_syscall_hook = hook;
        return ;
    }
    g_su_write_syscall_hook = ft_nullptr;
    return ;
}

void    su_reset_write_syscall_hook(void)
{
    g_su_write_syscall_hook = ft_nullptr;
    return ;
}

ssize_t su_read(int file_descriptor, void *buffer, size_t count)
{
    int retry_attempts = 0;
    while (true)
    {
        ssize_t read_result = cmp_read(file_descriptor, buffer, count);
        if (read_result >= 0)
        {
            ft_errno = ER_SUCCESS;
            return (read_result);
        }
        int stored_error = ft_errno;
#if defined(_WIN32) || defined(_WIN64)
        if (stored_error == ER_SUCCESS)
        {
            DWORD last_error = GetLastError();
            if (last_error != 0)
                stored_error = static_cast<int>(last_error) + ERRNO_OFFSET;
        }
#else
        if (stored_error == ER_SUCCESS && errno != 0)
            stored_error = errno + ERRNO_OFFSET;
#endif
        ft_errno = stored_error;
#if defined(__linux__) || defined(__APPLE__)
        const int max_retries = 10;
        const int retry_delay_ms = 500;
        if (ft_errno == EINTR + ERRNO_OFFSET)
            continue ;
        else if (ft_errno == EAGAIN + ERRNO_OFFSET
            || ft_errno == EWOULDBLOCK + ERRNO_OFFSET)
        {
            if (retry_attempts < max_retries)
            {
                retry_attempts++;
                struct timespec delay = {0, retry_delay_ms * 1000000L};
                nanosleep(&delay, ft_nullptr);
                continue ;
            }
            else
                return (-1);
        }
        else
            return (-1);
#else
        return (-1);
#endif
    }
}

ssize_t su_write(int file_descriptor, const void *buffer, size_t count)
{
    size_t total_written = 0;
    int retry_attempts = 0;
    const char *byte_buffer = static_cast<const char*>(buffer);
    t_su_write_syscall_hook write_function;

    write_function = g_su_write_syscall_hook;
    if (write_function == ft_nullptr)
        write_function = su_default_write_syscall;
    while (total_written < count)
    {
        ssize_t write_result = write_function(file_descriptor,
            byte_buffer + total_written, count - total_written);
        if (write_result > 0)
        {
            total_written += static_cast<size_t>(write_result);
        }
        else
        {
            if (write_result == 0)
            {
                ft_errno = FT_ERR_IO;
                return (-1);
            }
            int stored_error = ft_errno;
#if defined(_WIN32) || defined(_WIN64)
            if (stored_error == ER_SUCCESS)
            {
                DWORD last_error = GetLastError();
                if (last_error != 0)
                    stored_error = static_cast<int>(last_error) + ERRNO_OFFSET;
            }
#else
            if (stored_error == ER_SUCCESS && errno != 0)
                stored_error = errno + ERRNO_OFFSET;
#endif
            ft_errno = stored_error;
#if defined(__linux__) || defined(__APPLE__)
            const int max_retries = 10;
            const int retry_delay_ms = 500;
            if (ft_errno == EINTR + ERRNO_OFFSET)
                continue ;
            else if (ft_errno == EAGAIN + ERRNO_OFFSET
                || ft_errno == EWOULDBLOCK + ERRNO_OFFSET)
            {
                if (retry_attempts < max_retries)
                {
                    retry_attempts++;
                    struct timespec delay = {0, retry_delay_ms * 1000000L};
                    nanosleep(&delay, ft_nullptr);
                    continue ;
                }
                else
                    return (-1);
            }
            else
                return (-1);
#else
            return (-1);
#endif
        }
    }
    ft_errno = ER_SUCCESS;
    return (total_written);
}

int su_close(int file_descriptor)
{
    int close_result = cmp_close(file_descriptor);
    if (close_result != 0)
    {
        int stored_error = ft_errno;
#if defined(_WIN32) || defined(_WIN64)
        if (stored_error == ER_SUCCESS)
        {
            DWORD last_error = GetLastError();
            if (last_error != 0)
                stored_error = static_cast<int>(last_error) + ERRNO_OFFSET;
        }
#else
        if (stored_error == ER_SUCCESS && errno != 0)
            stored_error = errno + ERRNO_OFFSET;
#endif
        ft_errno = stored_error;
        return (close_result);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

