#include "logger_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cerrno>
#include <unistd.h>

static void log_close_report(int error_code)
{
    ft_errno = error_code;
    ft_global_error_stack_push(error_code);
    return ;
}

void ft_log_close()
{
    ft_log_enable_remote_health(false);
    size_t index;
    ft_vector<s_log_sink> sinks_snapshot;
    size_t sink_count;
    int    clear_error;
    int    final_error;

    if (logger_lock_sinks() != 0)
    {
        log_close_report(ft_errno);
        return ;
    }
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        if (logger_unlock_sinks() != 0)
        {
            log_close_report(ft_errno);
            return ;
        }
        log_close_report(final_error);
        return ;
    }
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            if (logger_unlock_sinks() != 0)
            {
                log_close_report(ft_errno);
                return ;
            }
            log_close_report(final_error);
            return ;
        }
        sinks_snapshot.push_back(entry);
        if (sinks_snapshot.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = sinks_snapshot.get_error();
            if (logger_unlock_sinks() != 0)
            {
                log_close_report(ft_errno);
                return ;
            }
            log_close_report(final_error);
            return ;
        }
        index++;
    }
    g_sinks.clear();
    clear_error = g_sinks.get_error();
    if (logger_unlock_sinks() != 0)
    {
        log_close_report(ft_errno);
        return ;
    }
    if (clear_error != FT_ERR_SUCCESSS)
    {
        log_close_report(clear_error);
        return ;
    }
    size_t snapshot_count;

    snapshot_count = sinks_snapshot.size();
    if (sinks_snapshot.get_error() != FT_ERR_SUCCESSS)
    {
        log_close_report(sinks_snapshot.get_error());
        return ;
    }
    index = 0;
    while (index < snapshot_count)
    {
        s_log_sink entry;

        entry = sinks_snapshot[index];
        if (sinks_snapshot.get_error() != FT_ERR_SUCCESSS)
        {
            log_close_report(sinks_snapshot.get_error());
            return ;
        }
        bool sink_lock_acquired;
        int  sink_error;

        sink_lock_acquired = false;
        sink_error = FT_ERR_SUCCESSS;
        if (log_sink_lock(&entry, &sink_lock_acquired) != 0)
            sink_error = ft_errno;
        else if (entry.function == ft_file_sink)
        {
            s_file_sink *sink;
            bool         file_sink_lock_acquired;

            sink = static_cast<s_file_sink *>(entry.user_data);
            if (sink)
            {
                file_sink_lock_acquired = false;
                if (file_sink_lock(sink, &file_sink_lock_acquired) != 0)
                    file_sink_lock_acquired = false;
                close(sink->fd);
                sink->fd = -1;
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
            bool            network_lock_acquired;

            sink = static_cast<s_network_sink *>(entry.user_data);
            if (sink)
            {
                network_lock_acquired = false;
                if (network_sink_lock(sink, &network_lock_acquired) != 0)
                    network_lock_acquired = false;
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
        if (sink_error != FT_ERR_SUCCESSS)
        {
            log_close_report(sink_error);
            return ;
        }
        index++;
    }
    log_close_report(FT_ERR_SUCCESSS);
    return ;
}
