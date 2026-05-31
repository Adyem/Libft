#include "dumb_io.hpp"

#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <errno.h>
#include <stdio.h>

int32_t ft_read_file(const char *path, char **out_buffer, ft_size_t *out_size)
{
    FILE    *file;
    int64_t file_size;
    char    *buffer;
    ft_size_t  bytes_read;
    int32_t system_error;

    if (path == ft_nullptr || out_buffer == ft_nullptr || out_size == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }

    *out_buffer = ft_nullptr;
    *out_size = 0;

    file = fopen(path, "rb");
    if (file == ft_nullptr)
    {
        system_error = cmp_map_system_error_to_ft(errno);
        if (system_error == FT_ERR_SUCCESS)
            system_error = FT_ERR_IO;
        return (system_error);
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0)
    {
        fclose(file);
        return (FT_ERR_IO);
    }

    if (file_size == 0)
    {
        *out_buffer = ft_nullptr;
        *out_size = 0;
        fclose(file);
        return (FT_ERR_SUCCESS);
    }
    buffer = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(file_size)));
    if (buffer == ft_nullptr)
    {
        fclose(file);
        return (FT_ERR_NO_MEMORY);
    }

    bytes_read = fread(buffer, 1, static_cast<ft_size_t>(file_size), file);
    if (bytes_read != static_cast<ft_size_t>(file_size))
    {
        cma_free(buffer);
        fclose(file);
        return (FT_ERR_IO);
    }

    fclose(file);

    *out_buffer = buffer;
    *out_size = static_cast<ft_size_t>(file_size);

    return (FT_ERR_SUCCESS);
}

int32_t ft_write_file(const char *path, const char *buffer, ft_size_t size)
{
    FILE    *file;
    ft_size_t  bytes_written;
    int32_t system_error;

    if (path == ft_nullptr || buffer == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }

    file = fopen(path, "wb");
    if (file == ft_nullptr)
    {
        system_error = cmp_map_system_error_to_ft(errno);
        if (system_error == FT_ERR_SUCCESS)
            system_error = FT_ERR_IO;
        return (system_error);
    }

    if (size > 0)
    {
        bytes_written = fwrite(buffer, 1, size, file);
        if (bytes_written != size)
        {
            fclose(file);
            return (FT_ERR_IO);
        }
    }

    fclose(file);
    return (FT_ERR_SUCCESS);
}
