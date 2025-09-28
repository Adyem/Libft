#include "logger_internal.hpp"
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <new>
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
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    file_descriptor = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (file_descriptor == -1)
    {
        int saved_errno;

        saved_errno = errno;
        if (saved_errno != 0)
            ft_errno = saved_errno + ERRNO_OFFSET;
        else
            ft_errno = FILE_INVALID_FD;
        return (-1);
    }
    sink = new(std::nothrow) s_file_sink;
    if (!sink)
    {
        close(file_descriptor);
        ft_errno = FT_EALLOC;
        return (-1);
    }
    sink->fd = file_descriptor;
    sink->path = path;
    sink->max_size = max_size;
    if (sink->path.get_error() != ER_SUCCESS)
    {
        ft_errno = sink->path.get_error();
        close(file_descriptor);
        delete sink;
        return (-1);
    }
    if (ft_log_add_sink(ft_file_sink, sink) != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_EINVAL;
        close(file_descriptor);
        delete sink;
        ft_errno = error_code;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
