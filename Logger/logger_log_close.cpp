#include "logger_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cerrno>
#include <unistd.h>

static void log_close_report(void)
{
    return ;
}

void ft_log_close()
{
    ft_log_enable_remote_health(FT_FALSE);
    ft_size_t entry_index;
    ft_vector<s_log_sink> sinks_snapshot;
    ft_size_t sink_count;
    int32_t    clear_error;
    int32_t    lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        log_close_report();
        return ;
    }
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESS)
    {
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESS)
        {
            log_close_report();
            return ;
        }
        log_close_report();
        return ;
    }
    entry_index = 0;
    while (entry_index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[entry_index];
        if (g_sinks.get_error() != FT_ERR_SUCCESS)
        {
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESS)
            {
                log_close_report();
                return ;
            }
            log_close_report();
            return ;
        }
        sinks_snapshot.push_back(entry);
        if (sinks_snapshot.get_error() != FT_ERR_SUCCESS)
        {
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESS)
            {
                log_close_report();
                return ;
            }
            log_close_report();
            return ;
        }
        entry_index++;
    }
    g_sinks.clear();
    clear_error = g_sinks.get_error();
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        log_close_report();
        return ;
    }
    if (clear_error != FT_ERR_SUCCESS)
    {
        log_close_report();
        return ;
    }
    ft_size_t snapshot_count;

    snapshot_count = sinks_snapshot.size();
    if (sinks_snapshot.get_error() != FT_ERR_SUCCESS)
    {
        log_close_report();
        return ;
    }
    entry_index = 0;
    while (entry_index < snapshot_count)
    {
        s_log_sink entry;

        entry = sinks_snapshot[entry_index];
        if (sinks_snapshot.get_error() != FT_ERR_SUCCESS)
        {
            log_close_report();
            return ;
        }
        ft_bool sink_lock_acquired;
        int32_t  sink_error;
        int32_t  sink_lock_error;

        sink_lock_acquired = FT_FALSE;
        sink_error = FT_ERR_SUCCESS;
        sink_lock_error = log_sink_lock(&entry, &sink_lock_acquired);
        if (sink_lock_error != FT_ERR_SUCCESS)
            sink_error = sink_lock_error;
        else if (entry.function == ft_file_sink)
        {
            s_file_sink *sink;
            ft_bool         file_sink_lock_acquired;
            int32_t          file_lock_error;

            sink = static_cast<s_file_sink *>(entry.user_data);
            if (sink)
            {
                file_sink_lock_acquired = FT_FALSE;
                file_lock_error = file_sink_lock(sink, &file_sink_lock_acquired);
                if (file_lock_error != FT_ERR_SUCCESS)
                    file_sink_lock_acquired = FT_FALSE;
                close(sink->file_descriptor);
                sink->file_descriptor = -1;
                if (file_sink_lock_acquired)
                    file_sink_unlock(sink, file_sink_lock_acquired);
                file_sink_teardown_thread_safety(sink);
                delete sink;
            }
        }
        else if (entry.function == ft_syslog_sink)
            cmp_syslog_close();
        else if (entry.function == ft_network_sink)
        {
            s_network_sink *sink;
            ft_bool            network_lock_acquired;
            int32_t             network_lock_error;

            sink = static_cast<s_network_sink *>(entry.user_data);
            if (sink)
            {
                network_lock_acquired = FT_FALSE;
                network_lock_error = network_sink_lock(sink, &network_lock_acquired);
                if (network_lock_error != FT_ERR_SUCCESS)
                    network_lock_acquired = FT_FALSE;
                cmp_close(sink->socket_fd);
                sink->socket_fd = -1;
                if (network_lock_acquired)
                    network_sink_unlock(sink, network_lock_acquired);
                network_sink_teardown_thread_safety(sink);
                delete sink;
            }
        }
        if (sink_lock_acquired)
            log_sink_unlock(&entry, sink_lock_acquired);
        log_sink_teardown_thread_safety(&entry);
        if (sink_error != FT_ERR_SUCCESS)
        {
            log_close_report();
            return ;
        }
        entry_index++;
    }
    log_close_report();
    return ;
}
