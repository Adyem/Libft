#include "class_file_stream.hpp"

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <climits>

ft_size_t ft_file_default_buffer_size(void) noexcept
{
    return (65536);
}

static int32_t ft_file_allocate_transfer_buffer(char **buffer_pointer,
                                                ft_size_t buffer_size) noexcept
{
    if (buffer_pointer == ft_nullptr)
        return (-1);
    if (buffer_size == 0)
    {
        *buffer_pointer = ft_nullptr;
        return (-1);
    }
    *buffer_pointer = static_cast<char *>(cma_malloc(buffer_size));
    if (*buffer_pointer == ft_nullptr)
        return (-1);
    return (0);
}

static void ft_file_release_transfer_buffer(char *buffer_pointer) noexcept
{
    if (buffer_pointer != ft_nullptr)
        cma_free(buffer_pointer);
    return ;
}

int32_t ft_file_stream_copy(ft_file &source_file, ft_file &destination_file,
                            ft_size_t buffer_size) noexcept
{
    char *transfer_buffer;
    ft_size_t maximum_readable;
    int32_t read_size;
    ssize_t bytes_read;
    ssize_t bytes_written;
    ft_size_t write_offset;

    if (buffer_size == 0)
        buffer_size = ft_file_default_buffer_size();
    if (ft_file_allocate_transfer_buffer(&transfer_buffer, buffer_size) != 0)
        return (-1);
    maximum_readable = buffer_size;
    if (maximum_readable > static_cast<ft_size_t>(INT_MAX))
        maximum_readable = static_cast<ft_size_t>(INT_MAX);
    read_size = static_cast<int32_t>(maximum_readable);
    while (FT_TRUE)
    {
        bytes_read = source_file.read(transfer_buffer, read_size);
        if (bytes_read < 0)
        {
            ft_file_release_transfer_buffer(transfer_buffer);
            return (-1);
        }
        if (bytes_read == 0)
            break ;
        write_offset = 0;
        while (write_offset < static_cast<ft_size_t>(bytes_read))
        {
            bytes_written = destination_file.write_buffer(transfer_buffer
                    + write_offset, static_cast<ft_size_t>(bytes_read) - write_offset);
            if (bytes_written < 0)
            {
                ft_file_release_transfer_buffer(transfer_buffer);
                return (-1);
            }
            write_offset += static_cast<ft_size_t>(bytes_written);
        }
    }
    ft_file_release_transfer_buffer(transfer_buffer);
    return (0);
}
