#include "system_utils.hpp"
#if defined(__linux__) || defined(__APPLE__)
# include <cerrno>
# include <ctime>
# include "../CPP_class/class_nullptr.hpp"
#endif

int su_open(const char *path_name)
{
    return (ft_open(path_name));
}

int su_open(const char *path_name, int flags)
{
    return (ft_open(path_name, flags));
}

int su_open(const char *path_name, int flags, mode_t mode)
{
    return (ft_open(path_name, flags, mode));
}

ssize_t su_read(int file_descriptor, void *buffer, size_t count)
{
    return (ft_read(file_descriptor, buffer, count));
}

ssize_t su_write(int file_descriptor, const void *buffer, size_t count)
{
    size_t total_written = 0;
    int retry_attempts = 0;
    const char *byte_buffer = static_cast<const char*>(buffer);
    while (total_written < count)
    {
        ssize_t bytes_written = ft_write(file_descriptor,
            byte_buffer + total_written, count - total_written);
        if (bytes_written >= 0)
        {
            total_written += bytes_written;
        }
#if defined(__linux__) || defined(__APPLE__)
        else
        {
            const int max_retries = 10;
            const int retry_delay_ms = 500;
            if (errno == EINTR)
                continue ;
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
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
        }
#else
        else
            return (-1);
#endif
    }
    return (total_written);
}

