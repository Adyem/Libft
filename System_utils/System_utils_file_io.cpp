#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>
#include <ctime>
#include <cstddef>

static t_su_write_syscall_hook    g_su_write_syscall_hook = ft_nullptr;

static ssize_t    su_default_write_syscall(int file_descriptor, const void *buffer, size_t count)
{
    int64_t bytes_written;
    int32_t error_code;

    bytes_written = 0;
    error_code = cmp_write(file_descriptor, buffer, count, &bytes_written);
    if (error_code != FT_ERR_SUCCESS)
        return (-1);
    return (static_cast<ssize_t>(bytes_written));
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
    int retry_attempts;
    int64_t bytes_read_value;

    retry_attempts = 0;
    while (true)
    {
        int32_t error_code;

        bytes_read_value = 0;
        error_code = cmp_read(file_descriptor, buffer, count, &bytes_read_value);
        if (error_code == FT_ERR_SUCCESS)
            return (static_cast<ssize_t>(bytes_read_value));
#if defined(__linux__) || defined(__APPLE__)
        const int32_t max_retries = 10;
        const int32_t retry_delay_ms = 500;
        if (error_code == cmp_map_system_error_to_ft(EINTR))
            continue ;
        else if (error_code == cmp_map_system_error_to_ft(EAGAIN)
            || error_code == cmp_map_system_error_to_ft(EWOULDBLOCK))
        {
            if (retry_attempts < static_cast<int>(max_retries))
            {
                retry_attempts++;
                struct timespec delay = {0, retry_delay_ms * 1000000L};
                nanosleep(&delay, ft_nullptr);
                continue ;
            }
            return (-1);
        }
        return (-1);
#else
        (void)error_code;
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
    int64_t bytes_written_value;

    total_written = 0;
    retry_attempts = 0;
    byte_buffer = static_cast<const char*>(buffer);
    write_function = g_su_write_syscall_hook;
    if (write_function == ft_nullptr)
        write_function = su_default_write_syscall;
    while (total_written < count)
    {
        ssize_t write_result;
        int32_t error_code;

        if (write_function == su_default_write_syscall)
        {
            bytes_written_value = 0;
            error_code = cmp_write(file_descriptor, byte_buffer + total_written,
                count - total_written, &bytes_written_value);
            if (error_code == FT_ERR_SUCCESS)
                write_result = static_cast<ssize_t>(bytes_written_value);
            else
                write_result = -1;
        }
        else
        {
            write_result = write_function(file_descriptor,
                byte_buffer + total_written, count - total_written);
            if (write_result < 0)
            {
                error_code = cmp_map_system_error_to_ft(errno);
                if (error_code == FT_ERR_SUCCESS)
                    error_code = FT_ERR_IO;
            }
        }
        if (write_result > 0)
            total_written += static_cast<size_t>(write_result);
        else
        {
            if (write_result == 0)
                return (-1);
#if defined(__linux__) || defined(__APPLE__)
            const int32_t max_retries = 10;
            const int32_t retry_delay_ms = 500;
            if (error_code == cmp_map_system_error_to_ft(EINTR))
                continue ;
            else if (error_code == cmp_map_system_error_to_ft(EAGAIN)
                || error_code == cmp_map_system_error_to_ft(EWOULDBLOCK))
            {
                if (retry_attempts < static_cast<int>(max_retries))
                {
                    retry_attempts++;
                    struct timespec delay = {0, retry_delay_ms * 1000000L};
                    nanosleep(&delay, ft_nullptr);
                    continue ;
                }
                return (-1);
            }
            return (-1);
#else
            (void)error_code;
            return (-1);
#endif
        }
    }
    return (total_written);
}

int su_close(int file_descriptor)
{
    int close_result;

    close_result = cmp_close(file_descriptor);
    if (close_result != 0)
        return (close_result);
    return (0);
}
