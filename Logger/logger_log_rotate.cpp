#include "logger_internal.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

void ft_log_rotate(s_file_sink *sink)
{
    struct stat st;
    ft_string   rotated;
    int         close_result;

    if (!sink)
    {
        ft_errno = FT_EINVAL;
        return ;
    }
    if (sink->path.empty() || sink->path.get_error() != ER_SUCCESS || sink->max_size == 0)
    {
        ft_errno = FT_EINVAL;
        return ;
    }
    if (fstat(sink->fd, &st) == -1)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return ;
    }
    if (static_cast<size_t>(st.st_size) < sink->max_size)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    close_result = close(sink->fd);
    if (close_result == -1)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return ;
    }
    rotated = sink->path + ".1";
    if (rotated.get_error() != ER_SUCCESS)
    {
        ft_errno = rotated.get_error();
        return ;
    }
    if (rename(sink->path.c_str(), rotated.c_str()) != 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return ;
    }
    sink->fd = open(sink->path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (sink->fd == -1)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}
