#include "logger_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Errno/errno.hpp"

static int32_t log_rotation_report(int32_t return_value)
{
    return (return_value);
}

int32_t ft_log_set_rotation(ft_size_t max_size, ft_size_t retention_count, uint32_t max_age_seconds)
{
    ft_size_t sink_count;
    ft_size_t entry_index;
    ft_bool   updated;
    int32_t    lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
        return (log_rotation_report(-1));
    sink_count = g_sinks.size();
    entry_index = 0;
    updated = FT_FALSE;
    while (entry_index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[entry_index];
        s_log_sink &stored_entry = g_sinks[entry_index];
        ft_bool        sink_lock_acquired;
        int32_t         sink_lock_error;

        sink_lock_acquired = FT_FALSE;
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
                ft_bool file_sink_lock_acquired;
                int32_t  file_lock_error;

                file_sink_lock_acquired = FT_FALSE;
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
                updated = FT_TRUE;
            }
        }
        if (sink_lock_acquired)
            log_sink_unlock(&stored_entry, sink_lock_acquired);
        entry_index += 1;
    }
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
        return (log_rotation_report(-1));
    if (!updated)
        return (log_rotation_report(-1));
    return (log_rotation_report(0));
}

int32_t ft_log_get_rotation(ft_size_t *max_size, ft_size_t *retention_count, uint32_t *max_age_seconds)
{
    ft_size_t sink_count;
    ft_size_t entry_index;
    int32_t    lock_error;

    if (!max_size || !retention_count || !max_age_seconds)
        return (log_rotation_report(-1));
    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
        return (log_rotation_report(-1));
    sink_count = g_sinks.size();
    entry_index = 0;
    while (entry_index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[entry_index];
        s_log_sink &stored_entry = g_sinks[entry_index];
        ft_bool        sink_lock_acquired;
        int32_t         sink_lock_error;

        sink_lock_acquired = FT_FALSE;
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
                ft_bool file_sink_lock_acquired;
                int32_t  file_lock_error;

                file_sink_lock_acquired = FT_FALSE;
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
        entry_index += 1;
    }
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
        return (log_rotation_report(-1));
    return (log_rotation_report(-1));
}
