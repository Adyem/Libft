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

static int log_set_file_report(int error_code, int return_value)
{
    ft_errno = error_code;
    ft_global_error_stack_push(error_code);
    return (return_value);
}

int ft_log_set_file(const char *path, size_t max_size)
{
    s_file_sink *sink;
    int          file_descriptor;

    if (!path)
        return (log_set_file_report(FT_ERR_INVALID_ARGUMENT, -1));
    file_descriptor = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (file_descriptor == -1)
    {
        int saved_errno;
        int error_code;

        saved_errno = errno;
        if (saved_errno != 0)
            error_code = cmp_map_system_error_to_ft(saved_errno);
        else
            error_code = FT_ERR_INVALID_HANDLE;
        return (log_set_file_report(error_code, -1));
    }
    sink = new(std::nothrow) s_file_sink;
    if (!sink)
    {
        close(file_descriptor);
        return (log_set_file_report(FT_ERR_NO_MEMORY, -1));
    }
    sink->fd = file_descriptor;
    sink->path = path;
    sink->max_size = max_size;
    sink->retention_count = 1;
    sink->max_age_seconds = 0;
    if (sink->path.get_error() != FT_ERR_SUCCESSS)
    {
        int error_code;

        error_code = sink->path.get_error();
        close(file_descriptor);
        delete sink;
        return (log_set_file_report(error_code, -1));
    }
    if (file_sink_prepare_thread_safety(sink) != 0)
    {
        int prepare_error;

        prepare_error = ft_errno;
        close(file_descriptor);
        delete sink;
        return (log_set_file_report(prepare_error, -1));
    }
    if (ft_log_add_sink(ft_file_sink, sink) != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        close(file_descriptor);
        file_sink_teardown_thread_safety(sink);
        delete sink;
        return (log_set_file_report(error_code, -1));
    }
    return (log_set_file_report(FT_ERR_SUCCESSS, 0));
}
