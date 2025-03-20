#include "linux_file.hpp"

ssize_t ft_write(int fd, const void *buffer, size_t count)
{
    size_t total_written = 0;
    int attempts = 0;

    while (total_written < count)
	{
        ssize_t bytes_written = write(fd, buffer + total_written, count - total_written);
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
				{
                    fprintf(stderr, "Error: Max retries reached (EAGAIN/EWOULDBLOCK)\n");
                    return (-1);
                }
            }
			else
			{
                fprintf(stderr, "Error: %s\n", strerror(errno));
                return (-1);
            }
        }
    }
    return (ssize_t)total_written;
}
