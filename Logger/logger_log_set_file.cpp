#include "logger_internal.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

void ft_file_sink(const char *message, void *user_data)
{
    s_file_sink *sink;
    size_t       length;
    ssize_t      written;

    sink = static_cast<s_file_sink *>(user_data);
    if (!sink)
        return ;
    length = std::strlen(message);
    written = write(sink->fd, message, length);
    (void)written;
    return ;
}

int ft_log_set_file(const char *path, size_t max_size)
{
    s_file_sink *sink;
    int          fd;

    if (!path)
        return (-1);
    fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd == -1)
        return (-1);
    sink = new s_file_sink;
    if (!sink)
    {
        close(fd);
        return (-1);
    }
    sink->fd = fd;
    sink->path = path;
    sink->max_size = max_size;
    if (ft_log_add_sink(ft_file_sink, sink) != 0)
    {
        close(fd);
        delete sink;
        return (-1);
    }
    return (0);
}
