#include "logger_internal.hpp"
#include <fcntl.h>
#include <unistd.h>

int ft_log_set_file(const char *path, size_t max_size)
{
    if (!path)
        return (-1);
    if (!g_path.empty())
        close(g_fd);
    int fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd == -1)
    {
        g_fd = 1;
        g_path.clear();
        g_max_size = 0;
        return (-1);
    }
    g_fd = fd;
    g_path = path;
    g_max_size = max_size;
    return (0);
}
