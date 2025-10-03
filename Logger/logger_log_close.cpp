#include "logger_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cerrno>
#include <unistd.h>

void ft_log_close()
{
    size_t index;
    ft_vector<s_log_sink> sinks_snapshot;
    size_t sink_count;
    int    clear_error;
    int    final_error;

    g_sinks_mutex.lock(THREAD_ID);
    if (g_sinks_mutex.get_error() != ER_SUCCESS)
        return ;
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != ER_SUCCESS)
    {
        final_error = g_sinks.get_error();
        g_sinks_mutex.unlock(THREAD_ID);
        if (g_sinks_mutex.get_error() != ER_SUCCESS)
            return ;
        ft_errno = final_error;
        return ;
    }
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != ER_SUCCESS)
        {
            final_error = g_sinks.get_error();
            g_sinks_mutex.unlock(THREAD_ID);
            if (g_sinks_mutex.get_error() != ER_SUCCESS)
                return ;
            ft_errno = final_error;
            return ;
        }
        sinks_snapshot.push_back(entry);
        if (sinks_snapshot.get_error() != ER_SUCCESS)
        {
            final_error = sinks_snapshot.get_error();
            g_sinks_mutex.unlock(THREAD_ID);
            if (g_sinks_mutex.get_error() != ER_SUCCESS)
                return ;
            ft_errno = final_error;
            return ;
        }
        index++;
    }
    g_sinks.clear();
    clear_error = g_sinks.get_error();
    g_sinks_mutex.unlock(THREAD_ID);
    if (g_sinks_mutex.get_error() != ER_SUCCESS)
        return ;
    if (clear_error != ER_SUCCESS)
    {
        ft_errno = clear_error;
        return ;
    }
    size_t snapshot_count;

    snapshot_count = sinks_snapshot.size();
    if (sinks_snapshot.get_error() != ER_SUCCESS)
    {
        ft_errno = sinks_snapshot.get_error();
        return ;
    }
    index = 0;
    while (index < snapshot_count)
    {
        s_log_sink entry;

        entry = sinks_snapshot[index];
        if (sinks_snapshot.get_error() != ER_SUCCESS)
        {
            ft_errno = sinks_snapshot.get_error();
            return ;
        }
        if (entry.function == ft_file_sink)
        {
            s_file_sink *sink;

            sink = static_cast<s_file_sink *>(entry.user_data);
            if (sink)
            {
                close(sink->fd);
                delete sink;
            }
        }
        else if (entry.function == ft_syslog_sink)
            cmp_syslog_close();
        else if (entry.function == ft_network_sink)
        {
            s_network_sink *sink;

            sink = static_cast<s_network_sink *>(entry.user_data);
            if (sink)
            {
                cmp_close(sink->socket_fd);
                delete sink;
            }
        }
        index++;
    }
    ft_errno = ER_SUCCESS;
    return ;
}
