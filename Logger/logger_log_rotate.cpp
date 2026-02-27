#include "logger_internal.hpp"
#include "../Basic/basic.hpp"
#include "../Time/time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static int logger_build_rotation_path(const ft_string &rotation_base,
    size_t index, ft_string &path)
{
    char index_buffer[32];

    std::snprintf(index_buffer, sizeof(index_buffer), "%zu", index);
    path = rotation_base + index_buffer;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

static int logger_remove_oldest_rotation(const ft_string &rotation_base,
    size_t retention_count)
{
    ft_string oldest_path;
    int unlink_result;
    int saved_errno;
    int normalized_error;

    if (logger_build_rotation_path(rotation_base, retention_count,
            oldest_path) != 0)
        return (-1);
    unlink_result = unlink(oldest_path.c_str());
    if (unlink_result == 0)
        return (0);
    saved_errno = errno;
    normalized_error = cmp_map_system_error_to_ft(saved_errno);
    if (normalized_error != FT_ERR_IO)
    {
        errno = saved_errno;
        return (-1);
    }
    errno = saved_errno;
    return (0);
}

static int logger_shift_rotation_chain(const ft_string &rotation_base,
    size_t retention_count)
{
    size_t current_index;
    ft_string source_path;
    ft_string destination_path;
    int saved_errno;
    int normalized_error;

    current_index = retention_count;
    while (current_index > 1)
    {
        if (logger_build_rotation_path(rotation_base, current_index - 1,
                source_path) != 0)
            return (-1);
        if (logger_build_rotation_path(rotation_base, current_index,
                destination_path) != 0)
            return (-1);
        if (::rename(source_path.c_str(), destination_path.c_str()) != 0)
        {
            saved_errno = errno;
            normalized_error = cmp_map_system_error_to_ft(saved_errno);
            if (normalized_error != FT_ERR_IO)
            {
                errno = saved_errno;
                return (-1);
            }
            errno = saved_errno;
        }
        current_index -= 1;
    }
    return (0);
}

static int logger_prepare_rotation_internal(s_file_sink *sink,
    bool *rotate_for_size, bool *rotate_for_age)
{
    struct stat file_stats;
    t_time current_time;
    long long age_seconds;
    size_t retention_count;

    if (!sink || !rotate_for_size || !rotate_for_age)
        return (-1);
    if (sink->path.empty())
        return (-1);
    if (sink->max_size == 0 && sink->max_age_seconds == 0)
    {
        *rotate_for_size = false;
        *rotate_for_age = false;
        return (0);
    }
    if (fstat(sink->file_descriptor, &file_stats) == -1)
        return (-1);
    retention_count = sink->retention_count;
    if (retention_count > 1024)
        retention_count = 0;
    if (retention_count == 0)
        retention_count = 1;
    sink->retention_count = retention_count;
    *rotate_for_size = false;
    if (sink->max_size > 0
        && static_cast<size_t>(file_stats.st_size) >= sink->max_size)
        *rotate_for_size = true;
    *rotate_for_age = false;
    if (sink->max_age_seconds > 0)
    {
        current_time = time_now();
        if (current_time == static_cast<t_time>(-1))
            return (-1);
        if (current_time >= file_stats.st_mtime)
        {
            age_seconds = static_cast<long long>(current_time)
                - static_cast<long long>(file_stats.st_mtime);
            if (age_seconds >= static_cast<long long>(sink->max_age_seconds))
                *rotate_for_age = true;
        }
    }
    return (0);
}

int logger_prepare_rotation(s_file_sink *sink, bool *rotate_for_size,
    bool *rotate_for_age)
{
    bool lock_acquired;
    int result;

    lock_acquired = false;
    if (file_sink_lock(sink, &lock_acquired) != 0)
        return (-1);
    result = logger_prepare_rotation_internal(sink, rotate_for_size,
            rotate_for_age);
    if (lock_acquired)
        file_sink_unlock(sink, lock_acquired);
    return (result);
}

void logger_execute_rotation(s_file_sink *sink)
{
    ft_string rotation_base;
    ft_string rotated_path;
    int close_result;
    int reopen_flags;
    size_t retention_count;
    bool lock_acquired;
    bool should_unlock;

    if (!sink)
        return ;
    lock_acquired = false;
    should_unlock = false;
    if (file_sink_lock(sink, &lock_acquired) != 0)
        return ;
    if (lock_acquired)
        should_unlock = true;
    retention_count = sink->retention_count;
    reopen_flags = O_CREAT | O_WRONLY | O_APPEND;
    if (retention_count > 0)
    {
        rotation_base = sink->path + ".";
        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
            goto cleanup;
        if (logger_remove_oldest_rotation(rotation_base, retention_count) != 0)
            goto cleanup;
        if (logger_shift_rotation_chain(rotation_base, retention_count) != 0)
            goto cleanup;
        rotated_path = sink->path + ".1";
        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
            goto cleanup;
        if (::rename(sink->path.c_str(), rotated_path.c_str()) != 0)
            goto cleanup;
    }
    else
    {
        reopen_flags = O_CREAT | O_WRONLY | O_TRUNC | O_APPEND;
    }
    close_result = close(sink->file_descriptor);
    if (close_result == -1)
    {
        sink->file_descriptor = -1;
        goto cleanup;
    }
    sink->file_descriptor = -1;
    sink->file_descriptor = open(sink->path.c_str(), reopen_flags, 0644);
    if (sink->file_descriptor == -1)
        goto cleanup;

cleanup:
    if (should_unlock)
        file_sink_unlock(sink, lock_acquired);
    return ;
}

void ft_log_rotate(s_file_sink *sink)
{
    bool rotate_for_size;
    bool rotate_for_age;

    rotate_for_size = false;
    rotate_for_age = false;
    if (logger_prepare_rotation(sink, &rotate_for_size, &rotate_for_age) != 0)
        return ;
    if (!rotate_for_size && !rotate_for_age)
        return ;
    logger_execute_rotation(sink);
    return ;
}
