#include "logger_internal.hpp"
#include <cerrno>

void ft_log_remove_sink(t_log_sink sink, void *user_data)
{
    size_t index;
    bool   removed;
    size_t sink_count;
    int    final_error;
    int    lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    index = 0;
    removed = false;
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(lock_error);
            return ;
        }
        ft_global_error_stack_push(final_error);
        return ;
    }
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(lock_error);
                return ;
            }
            ft_global_error_stack_push(final_error);
            return ;
        }
        if (entry.function == sink && entry.user_data == user_data)
        {
            s_log_sink &stored_entry = g_sinks[index];
            bool        sink_lock_acquired;
            int         sink_lock_error;

            if (g_sinks.get_error() != FT_ERR_SUCCESSS)
            {
                final_error = g_sinks.get_error();
                lock_error = logger_unlock_sinks();
                if (lock_error != FT_ERR_SUCCESSS)
                {
                    ft_global_error_stack_push(lock_error);
                    return ;
                }
                ft_global_error_stack_push(final_error);
                return ;
            }
            sink_lock_acquired = false;
            sink_lock_error = log_sink_lock(&stored_entry, &sink_lock_acquired);
            if (sink_lock_error != FT_ERR_SUCCESSS)
            {
                final_error = sink_lock_error;
                lock_error = logger_unlock_sinks();
                if (lock_error != FT_ERR_SUCCESSS)
                {
                    ft_global_error_stack_push(lock_error);
                    return ;
                }
                ft_global_error_stack_push(final_error);
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
                lock_error = logger_unlock_sinks();
                if (lock_error != FT_ERR_SUCCESSS)
                {
                    ft_global_error_stack_push(lock_error);
                    return ;
                }
                ft_global_error_stack_push(final_error);
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
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    ft_global_error_stack_push(final_error);
    return ;
}
