#if defined(__linux__) || defined(__APPLE__)

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include "linux_file.hpp"
#include <cstring>
#include <ctime>

int ft_open(const char* pathname)
{
    int fd = open(pathname, O_RDONLY);
    if (fd == -1)
        return (-1);
    return (fd);
}

int ft_open(const char* pathname, int flags)
{
    int fd = open(pathname, flags);
    if (fd == -1)
        return (-1);
    return (fd);
}

int ft_open(const char* pathname, int flags, mode_t mode)
{
    int fd = open(pathname, flags, mode);
    if (fd == -1)
        return (-1);
    return (fd);
}


ssize_t ft_read(int fd, void *buf, size_t count)
{
    ssize_t bytes_read = read(fd, buf, count);
    if (bytes_read == -1)
        return (-1);
    return (bytes_read);
}

int ft_close(int fd)
{
    if (close(fd) == -1)
        return (-1);
    return (0);
}

#endif
