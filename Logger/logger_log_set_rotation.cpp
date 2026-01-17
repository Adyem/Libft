#include "logger_internal.hpp"

static int log_rotation_report(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

int ft_log_set_rotation(size_t max_size, size_t retention_count, unsigned int max_age_seconds)
{
    size_t sink_count;
    size_t index;
    bool   updated;
    int    final_error;
    int    lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESSS)
        return (log_rotation_report(lock_error, -1));
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESSS)
            return (log_rotation_report(lock_error, -1));
        return (log_rotation_report(final_error, -1));
    }
    index = 0;
    updated = false;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESSS)
                return (log_rotation_report(lock_error, -1));
            return (log_rotation_report(final_error, -1));
        }
        s_log_sink &stored_entry = g_sinks[index];
        bool        sink_lock_acquired;
        int         sink_lock_error;

        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESSS)
                return (log_rotation_report(lock_error, -1));
            return (log_rotation_report(final_error, -1));
        }
        sink_lock_acquired = false;
        sink_lock_error = log_sink_lock(&stored_entry, &sink_lock_acquired);
        if (sink_lock_error != FT_ERR_SUCCESSS)
        {
            final_error = sink_lock_error;
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESSS)
                return (log_rotation_report(lock_error, -1));
            return (log_rotation_report(final_error, -1));
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
                if (file_lock_error != FT_ERR_SUCCESSS)
                {
                    final_error = file_lock_error;
                    if (sink_lock_acquired)
                        log_sink_unlock(&stored_entry, sink_lock_acquired);
                    lock_error = logger_unlock_sinks();
                    if (lock_error != FT_ERR_SUCCESSS)
                        return (log_rotation_report(lock_error, -1));
                    return (log_rotation_report(final_error, -1));
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
    if (lock_error != FT_ERR_SUCCESSS)
        return (log_rotation_report(lock_error, -1));
    if (!updated)
        return (log_rotation_report(FT_ERR_NOT_FOUND, -1));
    return (log_rotation_report(FT_ERR_SUCCESSS, 0));
}

int ft_log_get_rotation(size_t *max_size, size_t *retention_count, unsigned int *max_age_seconds)
{
    size_t sink_count;
    size_t index;
    int    final_error;
    int    lock_error;

    if (!max_size || !retention_count || !max_age_seconds)
        return (log_rotation_report(FT_ERR_INVALID_ARGUMENT, -1));
    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESSS)
        return (log_rotation_report(lock_error, -1));
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESSS)
            return (log_rotation_report(lock_error, -1));
        return (log_rotation_report(final_error, -1));
    }
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;

        entry = g_sinks[index];
        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESSS)
                return (log_rotation_report(lock_error, -1));
            return (log_rotation_report(final_error, -1));
        }
        s_log_sink &stored_entry = g_sinks[index];
        bool        sink_lock_acquired;
        int         sink_lock_error;

        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESSS)
                return (log_rotation_report(lock_error, -1));
            return (log_rotation_report(final_error, -1));
        }
        sink_lock_acquired = false;
        sink_lock_error = log_sink_lock(&stored_entry, &sink_lock_acquired);
        if (sink_lock_error != FT_ERR_SUCCESSS)
        {
            final_error = sink_lock_error;
            lock_error = logger_unlock_sinks();
            if (lock_error != FT_ERR_SUCCESSS)
                return (log_rotation_report(lock_error, -1));
            return (log_rotation_report(final_error, -1));
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
                if (file_lock_error != FT_ERR_SUCCESSS)
                {
                    final_error = file_lock_error;
                    if (sink_lock_acquired)
                        log_sink_unlock(&stored_entry, sink_lock_acquired);
                    lock_error = logger_unlock_sinks();
                    if (lock_error != FT_ERR_SUCCESSS)
                        return (log_rotation_report(lock_error, -1));
                    return (log_rotation_report(final_error, -1));
                }
                *max_size = file_sink->max_size;
                *retention_count = file_sink->retention_count;
                *max_age_seconds = file_sink->max_age_seconds;
                if (file_sink_lock_acquired)
                    file_sink_unlock(file_sink, file_sink_lock_acquired);
                if (sink_lock_acquired)
                    log_sink_unlock(&stored_entry, sink_lock_acquired);
                lock_error = logger_unlock_sinks();
                if (lock_error != FT_ERR_SUCCESSS)
                    return (log_rotation_report(lock_error, -1));
                return (log_rotation_report(FT_ERR_SUCCESSS, 0));
            }
        }
        if (sink_lock_acquired)
            log_sink_unlock(&stored_entry, sink_lock_acquired);
        index += 1;
    }
    lock_error = logger_unlock_sinks();
    if (lock_error != FT_ERR_SUCCESSS)
        return (log_rotation_report(lock_error, -1));
    return (log_rotation_report(FT_ERR_NOT_FOUND, -1));
}
