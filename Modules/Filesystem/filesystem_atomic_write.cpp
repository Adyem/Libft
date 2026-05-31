#include "filesystem.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../File/file_utils.hpp"
#include <cstdio>

static int32_t filesystem_write_all(FILE *file_stream, const void *data,
    ft_size_t size)
{
    const unsigned char *bytes;
    ft_size_t written_total;
    size_t written_now;

    if (file_stream == ft_nullptr || (data == ft_nullptr && size > 0))
    {
        return (FT_ERR_INVALID_POINTER);
    }
    bytes = static_cast<const unsigned char *>(data);
    written_total = 0;
    while (written_total < size)
    {
        written_now = std::fwrite(bytes + written_total, 1,
                size - written_total, file_stream);
        if (written_now == 0)
        {
            return (FT_ERR_IO);
        }
        written_total = written_total + written_now;
    }
    return (FT_ERR_SUCCESS);
}

int32_t filesystem_atomic_write(const char *path, const void *data, ft_size_t size)
{
    ft_string temporary_path;
    FILE *file_stream;
    int32_t error_code;
    int32_t close_error;

    if (path == ft_nullptr || (data == ft_nullptr && size > 0))
    {
        return (FT_ERR_INVALID_POINTER);
    }
    error_code = temporary_path.initialize(path);
    if (error_code != FT_ERR_SUCCESS)
    {
        return (error_code);
    }
    error_code = temporary_path.append(".tmp");
    if (error_code != FT_ERR_SUCCESS)
    {
        return (temporary_path.get_error());
    }
    file_stream = ft_fopen(temporary_path.c_str(), "wb");
    if (file_stream == ft_nullptr)
    {
        return (FT_ERR_FILE_OPEN_FAILED);
    }
    error_code = filesystem_write_all(file_stream, data, size);
    close_error = ft_fclose(file_stream);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)file_delete(temporary_path.c_str());
        return (error_code);
    }
    if (close_error != FT_ERR_SUCCESS)
    {
        (void)file_delete(temporary_path.c_str());
        return (close_error);
    }
    error_code = file_move(temporary_path.c_str(), path);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)file_delete(temporary_path.c_str());
    }
    return (error_code);
}
