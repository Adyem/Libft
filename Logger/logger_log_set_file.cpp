#include "logger_internal.hpp"
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <new>
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/basic.hpp"
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
    su_write(sink->file_descriptor, message, length);
    if (lock_acquired)
        file_sink_unlock(sink, lock_acquired);
    return ;
}

static int log_set_file_report(int return_value)
{
    return (return_value);
}

int ft_log_set_file(const char *path, size_t max_size)
{
    s_file_sink *sink;
    int          file_descriptor;
    int          prepare_error;

    if (!path)
        return (log_set_file_report(-1));
    file_descriptor = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (file_descriptor == -1)
        return (log_set_file_report(-1));
    sink = new(std::nothrow) s_file_sink;
    if (!sink)
    {
        close(file_descriptor);
        return (log_set_file_report(-1));
    }
    sink->file_descriptor = file_descriptor;
    sink->path = path;
    sink->max_size = max_size;
    sink->retention_count = 1;
    sink->max_age_seconds = 0;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
    {
        close(file_descriptor);
        delete sink;
        return (log_set_file_report(-1));
    }
    prepare_error = file_sink_prepare_thread_safety(sink);
    if (prepare_error != FT_ERR_SUCCESS)
    {
        close(file_descriptor);
        delete sink;
        return (log_set_file_report(-1));
    }
    if (ft_log_add_sink(ft_file_sink, sink) != 0)
    {
        close(file_descriptor);
        file_sink_teardown_thread_safety(sink);
        delete sink;
        return (log_set_file_report(-1));
    }
    return (log_set_file_report(0));
}
