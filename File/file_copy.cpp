#include "file_utils.hpp"

#include "../CPP_class/class_file.hpp"
#include "../CPP_class/class_file_stream.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <fcntl.h>
# define FILE_BINARY_FLAG O_BINARY
#else
# include <fcntl.h>
# define FILE_BINARY_FLAG 0
#endif

size_t file_default_copy_buffer_size(void) noexcept
{
    return (ft_file_default_buffer_size());
}

int file_copy_with_buffer(const char *source_path, const char *destination_path, size_t buffer_size)
{
    int source_flags;
    int destination_flags;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    ft_file source_file;
    ft_file destination_file;
    source_flags = O_RDONLY | FILE_BINARY_FLAG;
    if (source_file.open(source_path, source_flags) != 0)
        return (-1);
    destination_flags = O_WRONLY | O_CREAT | O_TRUNC | FILE_BINARY_FLAG;
    if (destination_file.open(destination_path, destination_flags, 0644) != 0)
        return (-1);
    if (ft_file_stream_copy(source_file, destination_file, buffer_size) != 0)
        return (-1);
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int file_copy(const char *source_path, const char *destination_path)
{
    return (file_copy_with_buffer(source_path, destination_path, file_default_copy_buffer_size()));
}
