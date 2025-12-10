#include "logger_internal.hpp"
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <new>
#include "../Compatebility/compatebility_internal.hpp"
#include "../Libft/libft.hpp"
#include "../System_utils/system_utils.hpp"

void ft_file_sink(const char *message, void *user_data)
{
    s_file_sink *sink;
    size_t       length;
    bool         lock_acquired;

    sink = static_cast<s_file_sink *>(user_data);
    if (!sink)
        return ;
    lock_acquired = false;
    if (file_sink_lock(sink, &lock_acquired) != 0)
        return ;
    length = ft_strlen(message);
    su_write(sink->fd, message, length);
    if (lock_acquired)
        file_sink_unlock(sink, lock_acquired);
    return ;
}

int ft_log_set_file(const char *path, size_t max_size)
{
    s_file_sink *sink;
    int          file_descriptor;

    if (!path)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    file_descriptor = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (file_descriptor == -1)
    {
        int saved_errno;

        saved_errno = errno;
        if (saved_errno != 0)
            ft_errno = cmp_map_system_error_to_ft(saved_errno);
        else
            ft_errno = FT_ERR_INVALID_HANDLE;
        return (-1);
    }
    sink = new(std::nothrow) s_file_sink;
    if (!sink)
    {
        close(file_descriptor);
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    sink->fd = file_descriptor;
    sink->path = path;
    sink->max_size = max_size;
    sink->retention_count = 1;
    sink->max_age_seconds = 0;
    if (sink->path.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = sink->path.get_error();
        close(file_descriptor);
        delete sink;
        return (-1);
    }
    if (file_sink_prepare_thread_safety(sink) != 0)
    {
        int prepare_error;

        prepare_error = ft_errno;
        close(file_descriptor);
        delete sink;
        ft_errno = prepare_error;
        return (-1);
    }
    if (ft_log_add_sink(ft_file_sink, sink) != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == FT_ER_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        close(file_descriptor);
        file_sink_teardown_thread_safety(sink);
        delete sink;
        ft_errno = error_code;
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}
