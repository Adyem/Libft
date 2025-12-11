#include "logger_internal.hpp"
#include <cerrno>

void ft_log_remove_sink(t_log_sink sink, void *user_data)
{
    size_t index;
    bool   removed;
    size_t sink_count;
    int    final_error;

    if (logger_lock_sinks() != 0)
        return ;
    index = 0;
    removed = false;
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        if (logger_unlock_sinks() != 0)
            return ;
        ft_errno = final_error;
        return ;
    }
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            if (logger_unlock_sinks() != 0)
                return ;
            ft_errno = final_error;
            return ;
        }
        if (entry.function == sink && entry.user_data == user_data)
        {
            s_log_sink &stored_entry = g_sinks[index];
            bool        sink_lock_acquired;

            if (g_sinks.get_error() != FT_ERR_SUCCESSS)
            {
                final_error = g_sinks.get_error();
                if (logger_unlock_sinks() != 0)
                    return ;
                ft_errno = final_error;
                return ;
            }
            sink_lock_acquired = false;
            if (log_sink_lock(&stored_entry, &sink_lock_acquired) != 0)
            {
                final_error = ft_errno;
                if (logger_unlock_sinks() != 0)
                    return ;
                ft_errno = final_error;
                return ;
            }
            s_log_sink locked_entry;

            locked_entry = stored_entry;
            g_sinks.erase(g_sinks.begin() + index);
            if (g_sinks.get_error() != FT_ERR_SUCCESSS)
            {
                final_error = g_sinks.get_error();
                if (sink_lock_acquired)
                    log_sink_unlock(&locked_entry, sink_lock_acquired);
                if (logger_unlock_sinks() != 0)
                    return ;
                ft_errno = final_error;
                return ;
            }
            if (sink_lock_acquired)
                log_sink_unlock(&locked_entry, sink_lock_acquired);
            log_sink_teardown_thread_safety(&locked_entry);
            removed = true;
            break;
        }
        index++;
    }
    if (!removed)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else
        final_error = FT_ERR_SUCCESSS;
    if (logger_unlock_sinks() != 0)
        return ;
    ft_errno = final_error;
    return ;
}
