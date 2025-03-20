#include "linux_file.hpp"
#include <cstdio>
#include <cerrno>
#include <ctime>

ssize_t ft_write(int fd, const void *buffer, size_t count)
{
    size_t total_written = 0;
    int attempts = 0;
    const char *buf = static_cast<const char*>(buffer);
    while (total_written < count)
    {
        ssize_t bytes_written = write(fd, buf + total_written, count - total_written);
        if (bytes_written >= 0)
            total_written += bytes_written;
        else
        {
            if (errno == EINTR)
                continue ;
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (attempts < MAX_RETRIES)
                {
                    attempts++;
                    struct timespec delay = {0, RETRY_DELAY_MS * 1000000L};
                    nanosleep(&delay, NULL);
                    continue ;
                }
                else
                    return (-1);
            }
            else
                return (-1);
        }
    }
    return (total_written);
}
