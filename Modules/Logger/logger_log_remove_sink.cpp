#include "logger_internal.hpp"
#include <cerrno>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"

void ft_log_remove_sink(t_log_sink sink, void *user_data)
{
    ft_size_t entry_index;
    ft_size_t sink_count;
    int32_t    lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    entry_index = 0;
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESS)
    {
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESS)
        {
            return ;
        }
        return ;
    }
    while (entry_index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[entry_index];
        if (g_sinks.get_error() != FT_ERR_SUCCESS)
        {
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESS)
            {
                return ;
            }
            return ;
        }
        if (entry.function == sink && entry.user_data == user_data)
        {
            s_log_sink &stored_entry = g_sinks[entry_index];
            ft_bool        sink_lock_acquired;
            int32_t         sink_lock_error;

            if (g_sinks.get_error() != FT_ERR_SUCCESS)
            {
                lock_error = logger_unlock_sinks();
                if (lock_error != FT_ERR_SUCCESS)
                {
                    return ;
                }
                return ;
            }
            sink_lock_acquired = FT_FALSE;
            sink_lock_error = log_sink_lock(&stored_entry, &sink_lock_acquired);
            if (sink_lock_error != FT_ERR_SUCCESS)
            {
                lock_error = logger_unlock_sinks();
                if (lock_error != FT_ERR_SUCCESS)
                {
                    return ;
                }
                return ;
            }
            s_log_sink locked_entry;

            locked_entry = stored_entry;
            g_sinks.erase(g_sinks.begin() + entry_index);
            if (g_sinks.get_error() != FT_ERR_SUCCESS)
            {
                if (sink_lock_acquired)
                    log_sink_unlock(&locked_entry, sink_lock_acquired);
                lock_error = logger_unlock_sinks();
                if (lock_error != FT_ERR_SUCCESS)
                {
                    return ;
                }
                return ;
            }
            if (sink_lock_acquired)
                log_sink_unlock(&locked_entry, sink_lock_acquired);
            log_sink_teardown_thread_safety(&locked_entry);
            break ;
        }
        entry_index++;
    }
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    return ;
}
