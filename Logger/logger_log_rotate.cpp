#include "logger_internal.hpp"
#include "../Basic/basic.hpp"
#include "../Time/time.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static int32_t logger_build_rotation_path(const ft_string &rotation_base,
    ft_size_t entry_index, ft_string &path)
{
    char index_buffer[32];

    std::snprintf(index_buffer, sizeof(index_buffer), "%zu", entry_index);
    path = rotation_base + index_buffer;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

static int32_t logger_remove_oldest_rotation(const ft_string &rotation_base,
    ft_size_t retention_count)
{
    ft_string oldest_path;
    int32_t unlink_result;
    int32_t saved_errno;
    int32_t normalized_error;

    if (logger_build_rotation_path(rotation_base, retention_count,
            oldest_path) != 0)
        return (FT_ERR_INTERNAL);
    unlink_result = unlink(oldest_path.c_str());
    if (unlink_result == 0)
        return (FT_ERR_SUCCESS);
    saved_errno = errno;
    normalized_error = cmp_map_system_error_to_ft(saved_errno);
    if (normalized_error != FT_ERR_IO)
    {
        errno = saved_errno;
        return (FT_ERR_INTERNAL);
    }
    errno = saved_errno;
    return (FT_ERR_SUCCESS);
}

static int32_t logger_shift_rotation_chain(const ft_string &rotation_base,
    ft_size_t retention_count)
{
    ft_size_t current_index;
    ft_string source_path;
    ft_string destination_path;
    int32_t saved_errno;
    int32_t normalized_error;

    current_index = retention_count;
    while (current_index > 1)
    {
        if (logger_build_rotation_path(rotation_base, current_index - 1,
                source_path) != 0)
            return (FT_ERR_INTERNAL);
        if (logger_build_rotation_path(rotation_base, current_index,
                destination_path) != 0)
            return (FT_ERR_INTERNAL);
        if (::rename(source_path.c_str(), destination_path.c_str()) != 0)
        {
            saved_errno = errno;
            normalized_error = cmp_map_system_error_to_ft(saved_errno);
            if (normalized_error != FT_ERR_IO)
            {
                errno = saved_errno;
                return (FT_ERR_INTERNAL);
            }
            errno = saved_errno;
        }
        current_index -= 1;
    }
    return (FT_ERR_SUCCESS);
}

static int32_t logger_prepare_rotation_internal(s_file_sink *sink,
    ft_bool *rotate_for_size, ft_bool *rotate_for_age)
{
    struct stat file_stats;
    t_time current_time;
    int64_t age_seconds;
    ft_size_t retention_count;

    if (!sink || !rotate_for_size || !rotate_for_age)
        return (FT_ERR_INTERNAL);
    if (sink->path.empty())
        return (FT_ERR_INTERNAL);
    if (sink->max_size == 0 && sink->max_age_seconds == 0)
    {
        *rotate_for_size = FT_FALSE;
        *rotate_for_age = FT_FALSE;
        return (FT_ERR_SUCCESS);
    }
    if (fstat(sink->file_descriptor, &file_stats) == -1)
        return (FT_ERR_INTERNAL);
    retention_count = sink->retention_count;
    if (retention_count > 1024)
        retention_count = 0;
    if (retention_count == 0)
        retention_count = 1;
    sink->retention_count = retention_count;
    *rotate_for_size = FT_FALSE;
    if (sink->max_size > 0
        && static_cast<ft_size_t>(file_stats.st_size) >= sink->max_size)
        *rotate_for_size = FT_TRUE;
    *rotate_for_age = FT_FALSE;
    if (sink->max_age_seconds > 0)
    {
        current_time = time_now();
        if (current_time == static_cast<t_time>(-1))
            return (FT_ERR_INTERNAL);
        if (current_time >= file_stats.st_mtime)
        {
            age_seconds = static_cast<int64_t>(current_time)
                - static_cast<int64_t>(file_stats.st_mtime);
            if (age_seconds >= static_cast<int64_t>(sink->max_age_seconds))
                *rotate_for_age = FT_TRUE;
        }
    }
    return (FT_ERR_SUCCESS);
}

int32_t logger_prepare_rotation(s_file_sink *sink, ft_bool *rotate_for_size,
    ft_bool *rotate_for_age)
{
    ft_bool lock_acquired;
    int32_t operation_result;

    lock_acquired = FT_FALSE;
    if (file_sink_lock(sink, &lock_acquired) != 0)
        return (FT_ERR_INTERNAL);
    operation_result = logger_prepare_rotation_internal(sink, rotate_for_size,
            rotate_for_age);
    if (lock_acquired)
        file_sink_unlock(sink, lock_acquired);
    return (operation_result);
}

void logger_execute_rotation(s_file_sink *sink)
{
    ft_string rotation_base;
    ft_string rotated_path;
    int32_t close_result;
    int32_t reopen_flags;
    ft_size_t retention_count;
    ft_bool lock_acquired;
    ft_bool should_unlock;

    if (!sink)
        return ;
    lock_acquired = FT_FALSE;
    should_unlock = FT_FALSE;
    if (file_sink_lock(sink, &lock_acquired) != 0)
        return ;
    if (lock_acquired)
        should_unlock = FT_TRUE;
    retention_count = sink->retention_count;
    reopen_flags = O_CREAT | O_WRONLY | O_APPEND;
    if (retention_count > 0)
    {
        rotation_base = sink->path + ".";
        if (ft_string::get_error() != FT_ERR_SUCCESS)
            goto cleanup;
        if (logger_remove_oldest_rotation(rotation_base, retention_count) != 0)
            goto cleanup;
        if (logger_shift_rotation_chain(rotation_base, retention_count) != 0)
            goto cleanup;
        rotated_path = sink->path + ".1";
        if (ft_string::get_error() != FT_ERR_SUCCESS)
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
    ft_bool rotate_for_size;
    ft_bool rotate_for_age;

    rotate_for_size = FT_FALSE;
    rotate_for_age = FT_FALSE;
    if (logger_prepare_rotation(sink, &rotate_for_size, &rotate_for_age) != 0)
        return ;
    if (!rotate_for_size && !rotate_for_age)
        return ;
    logger_execute_rotation(sink);
    return ;
}
