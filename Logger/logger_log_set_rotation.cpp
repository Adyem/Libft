#include "logger_internal.hpp"

static int log_rotation_report(int return_value)
{
    return (return_value);
}

int ft_log_set_rotation(size_t max_size, size_t retention_count, unsigned int max_age_seconds)
{
    size_t sink_count;
    size_t index;
    bool   updated;
    int    lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
        return (log_rotation_report(-1));
    sink_count = g_sinks.size();
    index = 0;
    updated = false;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        s_log_sink &stored_entry = g_sinks[index];
        bool        sink_lock_acquired;
        int         sink_lock_error;

        sink_lock_acquired = false;
        sink_lock_error = log_sink_lock(&stored_entry, &sink_lock_acquired);
        if (sink_lock_error != FT_ERR_SUCCESS)
        {
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESS)
                return (log_rotation_report(-1));
            return (log_rotation_report(-1));
        }
        entry = stored_entry;
        if (entry.function == ft_file_sink)
        {
            s_file_sink *file_sink;

            file_sink = static_cast<s_file_sink *>(entry.user_data);
            if (file_sink)
            {
                bool file_sink_lock_acquired;
                int  file_lock_error;

                file_sink_lock_acquired = false;
                file_lock_error = file_sink_lock(file_sink, &file_sink_lock_acquired);
                if (file_lock_error != FT_ERR_SUCCESS)
                {
                    if (sink_lock_acquired)
                        log_sink_unlock(&stored_entry, sink_lock_acquired);
                    lock_error = logger_unlock_sinks();
                    if (lock_error != FT_ERR_SUCCESS)
                        return (log_rotation_report(-1));
                    return (log_rotation_report(-1));
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
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
        return (log_rotation_report(-1));
    if (!updated)
        return (log_rotation_report(-1));
    return (log_rotation_report(0));
}

int ft_log_get_rotation(size_t *max_size, size_t *retention_count, unsigned int *max_age_seconds)
{
    size_t sink_count;
    size_t index;
    int    lock_error;

    if (!max_size || !retention_count || !max_age_seconds)
        return (log_rotation_report(-1));
    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
        return (log_rotation_report(-1));
    sink_count = g_sinks.size();
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        s_log_sink &stored_entry = g_sinks[index];
        bool        sink_lock_acquired;
        int         sink_lock_error;

        sink_lock_acquired = false;
        sink_lock_error = log_sink_lock(&stored_entry, &sink_lock_acquired);
        if (sink_lock_error != FT_ERR_SUCCESS)
        {
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESS)
                return (log_rotation_report(-1));
            return (log_rotation_report(-1));
        }
        entry = stored_entry;
        if (entry.function == ft_file_sink)
        {
            s_file_sink *file_sink;

            file_sink = static_cast<s_file_sink *>(entry.user_data);
            if (file_sink)
            {
                bool file_sink_lock_acquired;
                int  file_lock_error;

                file_sink_lock_acquired = false;
                file_lock_error = file_sink_lock(file_sink, &file_sink_lock_acquired);
                if (file_lock_error != FT_ERR_SUCCESS)
                {
                    if (sink_lock_acquired)
                        log_sink_unlock(&stored_entry, sink_lock_acquired);
                    lock_error = logger_unlock_sinks();
                    if (lock_error != FT_ERR_SUCCESS)
                        return (log_rotation_report(-1));
                    return (log_rotation_report(-1));
                }
                *max_size = file_sink->max_size;
                *retention_count = file_sink->retention_count;
                *max_age_seconds = file_sink->max_age_seconds;
                if (file_sink_lock_acquired)
                    file_sink_unlock(file_sink, file_sink_lock_acquired);
                if (sink_lock_acquired)
                    log_sink_unlock(&stored_entry, sink_lock_acquired);
                lock_error = logger_unlock_sinks();
                if (lock_error != FT_ERR_SUCCESS)
                    return (log_rotation_report(-1));
                return (log_rotation_report(0));
            }
        }
        if (sink_lock_acquired)
            log_sink_unlock(&stored_entry, sink_lock_acquired);
        index += 1;
    }
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
        return (log_rotation_report(-1));
    return (log_rotation_report(-1));
}
