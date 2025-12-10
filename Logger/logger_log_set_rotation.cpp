#include "logger_internal.hpp"

int ft_log_set_rotation(size_t max_size, size_t retention_count, unsigned int max_age_seconds)
{
    size_t sink_count;
    size_t index;
    bool   updated;
    int    final_error;

    if (logger_lock_sinks() != 0)
        return (-1);
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ER_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        if (logger_unlock_sinks() != 0)
            return (-1);
        ft_errno = final_error;
        return (-1);
    }
    index = 0;
    updated = false;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != FT_ER_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            if (logger_unlock_sinks() != 0)
                return (-1);
            ft_errno = final_error;
            return (-1);
        }
        s_log_sink &stored_entry = g_sinks[index];
        bool        sink_lock_acquired;

        if (g_sinks.get_error() != FT_ER_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            if (logger_unlock_sinks() != 0)
                return (-1);
            ft_errno = final_error;
            return (-1);
        }
        sink_lock_acquired = false;
        if (log_sink_lock(&stored_entry, &sink_lock_acquired) != 0)
        {
            final_error = ft_errno;
            if (logger_unlock_sinks() != 0)
                return (-1);
            ft_errno = final_error;
            return (-1);
        }
        entry = stored_entry;
        if (entry.function == ft_file_sink)
        {
            s_file_sink *file_sink;

            file_sink = static_cast<s_file_sink *>(entry.user_data);
            if (file_sink)
            {
                bool file_sink_lock_acquired;

                file_sink_lock_acquired = false;
                if (file_sink_lock(file_sink, &file_sink_lock_acquired) != 0)
                {
                    final_error = ft_errno;
                    if (sink_lock_acquired)
                        log_sink_unlock(&stored_entry, sink_lock_acquired);
                    if (logger_unlock_sinks() != 0)
                        return (-1);
                    ft_errno = final_error;
                    return (-1);
                }
                file_sink->max_size = max_size;
                file_sink->retention_count = retention_count;
                file_sink->max_age_seconds = max_age_seconds;
                if (file_sink_lock_acquired)
                    file_sink_unlock(file_sink, file_sink_lock_acquired);
                updated = true;
            }
        }
        if (sink_lock_acquired)
            log_sink_unlock(&stored_entry, sink_lock_acquired);
        index += 1;
    }
    if (logger_unlock_sinks() != 0)
        return (-1);
    if (!updated)
    {
        ft_errno = FT_ERR_NOT_FOUND;
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int ft_log_get_rotation(size_t *max_size, size_t *retention_count, unsigned int *max_age_seconds)
{
    size_t sink_count;
    size_t index;
    int    final_error;

    if (!max_size || !retention_count || !max_age_seconds)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (logger_lock_sinks() != 0)
        return (-1);
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ER_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        if (logger_unlock_sinks() != 0)
            return (-1);
        ft_errno = final_error;
        return (-1);
    }
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != FT_ER_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            if (logger_unlock_sinks() != 0)
                return (-1);
            ft_errno = final_error;
            return (-1);
        }
        s_log_sink &stored_entry = g_sinks[index];
        bool        sink_lock_acquired;

        if (g_sinks.get_error() != FT_ER_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            if (logger_unlock_sinks() != 0)
                return (-1);
            ft_errno = final_error;
            return (-1);
        }
        sink_lock_acquired = false;
        if (log_sink_lock(&stored_entry, &sink_lock_acquired) != 0)
        {
            final_error = ft_errno;
            if (logger_unlock_sinks() != 0)
                return (-1);
            ft_errno = final_error;
            return (-1);
        }
        entry = stored_entry;
        if (entry.function == ft_file_sink)
        {
            s_file_sink *file_sink;

            file_sink = static_cast<s_file_sink *>(entry.user_data);
            if (file_sink)
            {
                bool file_sink_lock_acquired;

                file_sink_lock_acquired = false;
                if (file_sink_lock(file_sink, &file_sink_lock_acquired) != 0)
                {
                    final_error = ft_errno;
                    if (sink_lock_acquired)
                        log_sink_unlock(&stored_entry, sink_lock_acquired);
                    if (logger_unlock_sinks() != 0)
                        return (-1);
                    ft_errno = final_error;
                    return (-1);
                }
                *max_size = file_sink->max_size;
                *retention_count = file_sink->retention_count;
                *max_age_seconds = file_sink->max_age_seconds;
                if (file_sink_lock_acquired)
                    file_sink_unlock(file_sink, file_sink_lock_acquired);
                if (sink_lock_acquired)
                    log_sink_unlock(&stored_entry, sink_lock_acquired);
                if (logger_unlock_sinks() != 0)
                    return (-1);
                ft_errno = FT_ER_SUCCESSS;
                return (0);
            }
        }
        if (sink_lock_acquired)
            log_sink_unlock(&stored_entry, sink_lock_acquired);
        index += 1;
    }
    if (logger_unlock_sinks() != 0)
        return (-1);
    ft_errno = FT_ERR_NOT_FOUND;
    return (-1);
}
