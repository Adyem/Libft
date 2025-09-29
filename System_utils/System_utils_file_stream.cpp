#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdlib>

static bool g_force_file_stream_allocation_failure = false;

void su_force_file_stream_allocation_failure(bool should_fail)
{
    g_force_file_stream_allocation_failure = should_fail;
    return ;
}

static su_file *create_file_stream(int file_descriptor)
{
    su_file *file_stream;

    if (file_descriptor < 0)
        return (ft_nullptr);
    if (g_force_file_stream_allocation_failure == true)
    {
        cmp_close(file_descriptor);
        ft_errno = FT_EALLOC;
        return (ft_nullptr);
    }
    file_stream = static_cast<su_file*>(std::malloc(sizeof(su_file)));
    if (file_stream == ft_nullptr)
    {
        cmp_close(file_descriptor);
        ft_errno = FT_EALLOC;
        return (ft_nullptr);
    }
    file_stream->_descriptor = file_descriptor;
    ft_errno = ER_SUCCESS;
    return (file_stream);
}

su_file *su_fopen(const char *path_name)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name);
    return (create_file_stream(file_descriptor));
}

su_file *su_fopen(const char *path_name, int flags)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name, flags);
    return (create_file_stream(file_descriptor));
}

su_file *su_fopen(const char *path_name, int flags, mode_t mode)
{
    int file_descriptor;

    if (path_name == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    file_descriptor = su_open(path_name, flags, mode);
    return (create_file_stream(file_descriptor));
}

int su_fclose(su_file *stream)
{
    int result;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    result = cmp_close(stream->_descriptor);
    if (result == 0)
    {
        std::free(stream);
        return (0);
    }
    return (result);
}

size_t su_fread(void *buffer, size_t size, size_t count, su_file *stream)
{
    size_t total_size;
    size_t total_read;
    char *byte_buffer;
    ssize_t bytes_read;

    if (buffer == ft_nullptr || stream == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    total_size = size * count;
    total_read = 0;
    byte_buffer = static_cast<char*>(buffer);
    while (total_read < total_size)
    {
        bytes_read = su_read(stream->_descriptor,
            byte_buffer + total_read, total_size - total_read);
        if (bytes_read <= 0)
            break;
        total_read += static_cast<size_t>(bytes_read);
    }
    return (total_read / size);
}

size_t su_fwrite(const void *buffer, size_t size, size_t count, su_file *stream)
{
    size_t total_size;
    ssize_t bytes_written;

    if (buffer == ft_nullptr || stream == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    total_size = size * count;
    bytes_written = su_write(stream->_descriptor, buffer, total_size);
    if (bytes_written < 0)
        return (0);
    return (static_cast<size_t>(bytes_written) / size);
}

int su_fseek(su_file *stream, long offset, int origin)
{
    off_t result;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    result = lseek(stream->_descriptor, offset, origin);
    if (result < 0)
        return (-1);
    return (0);
}

long su_ftell(su_file *stream)
{
    off_t position;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1L);
    }
    position = lseek(stream->_descriptor, 0, SEEK_CUR);
    if (position < 0)
        return (-1L);
    return (position);
}

