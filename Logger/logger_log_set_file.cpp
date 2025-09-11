#include "logger_internal.hpp"
#include <fcntl.h>
#include <unistd.h>
#include "../Libft/libft.hpp"

void ft_file_sink(const char *message, void *user_data)
{
    s_file_sink *sink;
    size_t       length;
    ssize_t      written;

    sink = static_cast<s_file_sink *>(user_data);
    if (!sink)
        return ;
    length = ft_strlen(message);
    written = write(sink->fd, message, length);
    (void)written;
    return ;
}

int ft_log_set_file(const char *path, size_t max_size)
{
    s_file_sink *sink;
    int          file_descriptor;

    if (!path)
        return (-1);
    file_descriptor = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (file_descriptor == -1)
        return (-1);
    sink = new s_file_sink;
    if (!sink)
    {
        close(file_descriptor);
        return (-1);
    }
    sink->fd = file_descriptor;
    sink->path = path;
    sink->max_size = max_size;
    if (sink->path.get_error() != ER_SUCCESS ||
        ft_log_add_sink(ft_file_sink, sink) != 0)
    {
        close(file_descriptor);
        delete sink;
        return (-1);
    }
    return (0);
}
