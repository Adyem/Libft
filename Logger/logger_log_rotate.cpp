#include "logger_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Time/time.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static int logger_remove_oldest_rotation(const ft_string &rotation_base, size_t retention_count)
{
    ft_string oldest_index;
    ft_string oldest_path;
    int unlink_result;

    oldest_index = ft_to_string(static_cast<long>(retention_count));
    if (oldest_index.get_error() != ER_SUCCESS)
    {
        ft_errno = oldest_index.get_error();
        return (-1);
    }
    oldest_path = rotation_base + oldest_index;
    if (oldest_path.get_error() != ER_SUCCESS)
    {
        ft_errno = oldest_path.get_error();
        return (-1);
    }
    unlink_result = unlink(oldest_path.c_str());
    if (unlink_result != 0)
    {
        int saved_errno;

        saved_errno = errno;
        if (saved_errno != ENOENT)
        {
            ft_errno_reference() = saved_errno + ERRNO_OFFSET;
            errno = saved_errno;
            return (-1);
        }
        errno = saved_errno;
    }
    return (0);
}

static int logger_shift_rotation_chain(const ft_string &rotation_base, size_t retention_count)
{
    size_t current_index;

    current_index = retention_count;
    while (current_index > 1)
    {
        ft_string source_index;
        ft_string destination_index;
        ft_string source_path;
        ft_string destination_path;

        source_index = ft_to_string(static_cast<long>(current_index - 1));
        if (source_index.get_error() != ER_SUCCESS)
        {
            ft_errno = source_index.get_error();
            return (-1);
        }
        destination_index = ft_to_string(static_cast<long>(current_index));
        if (destination_index.get_error() != ER_SUCCESS)
        {
            ft_errno = destination_index.get_error();
            return (-1);
        }
        source_path = rotation_base + source_index;
        if (source_path.get_error() != ER_SUCCESS)
        {
            ft_errno = source_path.get_error();
            return (-1);
        }
        destination_path = rotation_base + destination_index;
        if (destination_path.get_error() != ER_SUCCESS)
        {
            ft_errno = destination_path.get_error();
            return (-1);
        }
        if (rename(source_path.c_str(), destination_path.c_str()) != 0)
        {
            int saved_errno;

            saved_errno = errno;
            if (saved_errno != ENOENT)
            {
                ft_errno_reference() = saved_errno + ERRNO_OFFSET;
                errno = saved_errno;
                return (-1);
            }
            errno = saved_errno;
        }
        current_index -= 1;
    }
    return (0);
}

static int logger_prepare_rotation_internal(s_file_sink *sink, bool *rotate_for_size, bool *rotate_for_age)
{
    struct stat file_stats;
    t_time current_time;
    long long age_seconds;
    size_t retention_count;

    if (!sink)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (sink->path.empty() || sink->path.get_error() != ER_SUCCESS)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!rotate_for_size || !rotate_for_age)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (sink->max_size == 0 && sink->max_age_seconds == 0)
    {
        *rotate_for_size = false;
        *rotate_for_age = false;
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (fstat(sink->fd, &file_stats) == -1)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    retention_count = sink->retention_count;
    if (retention_count > 1024)
        retention_count = 0;
    if (retention_count == 0)
        retention_count = 1;
    sink->retention_count = retention_count;
    *rotate_for_size = false;
    if (sink->max_size > 0)
    {
        if (static_cast<size_t>(file_stats.st_size) >= sink->max_size)
            *rotate_for_size = true;
    }
    *rotate_for_age = false;
    if (sink->max_age_seconds > 0)
    {
        current_time = time_now();
        if (current_time == static_cast<t_time>(-1))
            return (-1);
        if (current_time >= static_cast<t_time>(file_stats.st_mtime))
        {
            age_seconds = static_cast<long long>(current_time) - static_cast<long long>(file_stats.st_mtime);
            if (age_seconds >= static_cast<long long>(sink->max_age_seconds))
                *rotate_for_age = true;
        }
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int logger_prepare_rotation(s_file_sink *sink, bool *rotate_for_size, bool *rotate_for_age)
{
    return (logger_prepare_rotation_internal(sink, rotate_for_size, rotate_for_age));
}

void logger_execute_rotation(s_file_sink *sink)
{
    ft_string rotation_base;
    ft_string rotated_path;
    int close_result;
    int saved_errno;
    int reopen_flags;
    size_t retention_count;

    if (!sink)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    retention_count = sink->retention_count;
    reopen_flags = O_CREAT | O_WRONLY | O_APPEND;
    if (retention_count > 0)
    {
        rotation_base = sink->path + ".";
        if (rotation_base.get_error() != ER_SUCCESS)
        {
            ft_errno = rotation_base.get_error();
            return ;
        }
        if (logger_remove_oldest_rotation(rotation_base, retention_count) != 0)
            return ;
        if (logger_shift_rotation_chain(rotation_base, retention_count) != 0)
            return ;
        rotated_path = sink->path + ".1";
        if (rotated_path.get_error() != ER_SUCCESS)
        {
            ft_errno = rotated_path.get_error();
            return ;
        }
        if (rename(sink->path.c_str(), rotated_path.c_str()) != 0)
        {
            saved_errno = errno;
            ft_errno_reference() = saved_errno + ERRNO_OFFSET;
            errno = saved_errno;
            return ;
        }
    }
    else
    {
        reopen_flags = O_CREAT | O_WRONLY | O_TRUNC | O_APPEND;
    }
    close_result = close(sink->fd);
    if (close_result == -1)
    {
        sink->fd = -1;
        saved_errno = errno;
        ft_errno_reference() = saved_errno + ERRNO_OFFSET;
        errno = saved_errno;
        return ;
    }
    sink->fd = -1;
    sink->fd = open(sink->path.c_str(), reopen_flags, 0644);
    if (sink->fd == -1)
    {
        saved_errno = errno;
        ft_errno_reference() = saved_errno + ERRNO_OFFSET;
        errno = saved_errno;
        return ;
    }
    ft_errno = ER_SUCCESS;
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
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    logger_execute_rotation(sink);
    return ;
}
