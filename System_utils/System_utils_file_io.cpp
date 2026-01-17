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
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    g_su_write_syscall_hook = ft_nullptr;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

void    su_reset_write_syscall_hook(void)
{
    g_su_write_syscall_hook = ft_nullptr;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ssize_t su_read(int file_descriptor, void *buffer, size_t count)
{
    int retry_attempts;

    retry_attempts = 0;
    while (true)
    {
        ssize_t read_result;
        int error_code;

        read_result = cmp_read(file_descriptor, buffer, count);
        if (read_result >= 0)
        {
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (read_result);
        }
        error_code = FT_ERR_IO;
#if defined(_WIN32) || defined(_WIN64)
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            error_code = ft_map_system_error(static_cast<int>(last_error));
#else
        if (errno != 0)
            error_code = ft_map_system_error(errno);
#endif
#if defined(__linux__) || defined(__APPLE__)
        const int max_retries = 10;
        const int retry_delay_ms = 500;
        if (error_code == ft_map_system_error(EINTR))
            continue ;
        else if (error_code == ft_map_system_error(EAGAIN)
            || error_code == ft_map_system_error(EWOULDBLOCK))
        {
            if (retry_attempts < max_retries)
            {
                retry_attempts++;
                struct timespec delay = {0, retry_delay_ms * 1000000L};
                nanosleep(&delay, ft_nullptr);
                continue ;
            }
            else
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
        }
        else
        {
            ft_global_error_stack_push(error_code);
            return (-1);
        }
#else
        ft_global_error_stack_push(error_code);
        return (-1);
#endif
    }
}

ssize_t su_write(int file_descriptor, const void *buffer, size_t count)
{
    size_t total_written;
    int retry_attempts;
    const char *byte_buffer;
    t_su_write_syscall_hook write_function;

    total_written = 0;
    retry_attempts = 0;
    byte_buffer = static_cast<const char*>(buffer);
    write_function = g_su_write_syscall_hook;
    if (write_function == ft_nullptr)
        write_function = su_default_write_syscall;
    while (total_written < count)
    {
        ssize_t write_result;

        write_result = write_function(file_descriptor,
            byte_buffer + total_written, count - total_written);
        if (write_result > 0)
        {
            total_written += static_cast<size_t>(write_result);
        }
        else
        {
            if (write_result == 0)
            {
                ft_global_error_stack_push(FT_ERR_IO);
                return (-1);
            }
            int error_code;

            error_code = FT_ERR_IO;
#if defined(_WIN32) || defined(_WIN64)
            DWORD last_error;

            last_error = GetLastError();
            if (last_error != 0)
                error_code = ft_map_system_error(static_cast<int>(last_error));
#else
            if (errno != 0)
                error_code = ft_map_system_error(errno);
#endif
#if defined(__linux__) || defined(__APPLE__)
            const int max_retries = 10;
            const int retry_delay_ms = 500;
            if (error_code == ft_map_system_error(EINTR))
                continue ;
            else if (error_code == ft_map_system_error(EAGAIN)
                || error_code == ft_map_system_error(EWOULDBLOCK))
            {
                if (retry_attempts < max_retries)
                {
                    retry_attempts++;
                    struct timespec delay = {0, retry_delay_ms * 1000000L};
                    nanosleep(&delay, ft_nullptr);
                    continue ;
                }
                else
                {
                    ft_global_error_stack_push(error_code);
                    return (-1);
                }
            }
            else
            {
                ft_global_error_stack_push(error_code);
                return (-1);
            }
#else
            ft_global_error_stack_push(error_code);
            return (-1);
#endif
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (total_written);
}

int su_close(int file_descriptor)
{
    int close_result;

    close_result = cmp_close(file_descriptor);
    if (close_result != 0)
    {
        int error_code;

        error_code = FT_ERR_IO;
#if defined(_WIN32) || defined(_WIN64)
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            error_code = ft_map_system_error(static_cast<int>(last_error));
#else
        if (errno != 0)
            error_code = ft_map_system_error(errno);
#endif
        ft_global_error_stack_push(error_code);
        return (close_result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
