#include "dumb_io.hpp"

#include <stdio.h>
#include <stdlib.h>

int ft_read_file(const char *path, char **out_buffer, size_t *out_size)
{
    FILE    *file;
    long    file_size;
    char    *buffer;
    size_t  bytes_read;

    if (path == NULL || out_buffer == NULL || out_size == NULL)
    {
        return (ft_io_error_invalid_argument);
    }

    *out_buffer = NULL;
    *out_size = 0;

    file = fopen(path, "rb");
    if (file == NULL)
    {
        return (ft_io_error_file_not_found);
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0)
    {
        fclose(file);
        return (ft_io_error_read_failed);
    }

    buffer = (char *)malloc((size_t)file_size);
    if (buffer == NULL)
    {
        fclose(file);
        return (ft_io_error_read_failed);
    }

    bytes_read = fread(buffer, 1, (size_t)file_size, file);
    if (bytes_read != (size_t)file_size)
    {
        free(buffer);
        fclose(file);
        return (ft_io_error_read_failed);
    }

    fclose(file);

    *out_buffer = buffer;
    *out_size = (size_t)file_size;

    return (ft_io_ok);
}

int ft_write_file(const char *path, const char *buffer, size_t size)
{
    FILE    *file;
    size_t  bytes_written;

    if (path == NULL || buffer == NULL)
    {
        return (ft_io_error_invalid_argument);
    }

    file = fopen(path, "wb");
    if (file == NULL)
    {
        return (ft_io_error_permission_denied);
    }

    if (size > 0)
    {
        bytes_written = fwrite(buffer, 1, size, file);
        if (bytes_written != size)
        {
            fclose(file);
            return (ft_io_error_write_failed);
        }
    }

    fclose(file);
    return (ft_io_ok);
}
