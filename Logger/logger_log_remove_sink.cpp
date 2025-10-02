#include "logger_internal.hpp"
#include <cerrno>

void ft_log_remove_sink(t_log_sink sink, void *user_data)
{
    size_t index;
    bool   removed;
    size_t sink_count;
    int    final_error;

    g_sinks_mutex.lock(THREAD_ID);
    if (g_sinks_mutex.get_error() != ER_SUCCESS)
    {
        return ;
    }
    index = 0;
    removed = false;
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != ER_SUCCESS)
    {
        final_error = g_sinks.get_error();
        g_sinks_mutex.unlock(THREAD_ID);
        if (g_sinks_mutex.get_error() != ER_SUCCESS)
        {
            return ;
        }
        ft_errno = final_error;
        return ;
    }
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != ER_SUCCESS)
        {
            final_error = g_sinks.get_error();
            g_sinks_mutex.unlock(THREAD_ID);
            if (g_sinks_mutex.get_error() != ER_SUCCESS)
            {
                return ;
            }
            ft_errno = final_error;
            return ;
        }
        if (entry.function == sink && entry.user_data == user_data)
        {
            g_sinks.erase(g_sinks.begin() + index);
            if (g_sinks.get_error() != ER_SUCCESS)
            {
                final_error = g_sinks.get_error();
                g_sinks_mutex.unlock(THREAD_ID);
                if (g_sinks_mutex.get_error() != ER_SUCCESS)
                {
                    return ;
                }
                ft_errno = final_error;
                return ;
            }
            removed = true;
            break;
        }
        index++;
    }
    if (!removed)
        final_error = FT_EINVAL;
    else
        final_error = ER_SUCCESS;
    g_sinks_mutex.unlock(THREAD_ID);
    if (g_sinks_mutex.get_error() != ER_SUCCESS)
    {
        return ;
    }
    ft_errno = final_error;
    return ;
}
