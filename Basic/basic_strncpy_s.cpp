#include <cstddef>
#include "basic.hpp"

static void zero_buffer(char *buffer, ft_size_t buffer_size)
{
    if (!buffer || buffer_size == 0)
        return ;
    ft_size_t index = 0;
    while (index < buffer_size)
    {
        buffer[index] = '\0';
        ++index;
    }
}

int32_t ft_strncpy_s(char *destination, ft_size_t destination_size,
                        const char *source, ft_size_t maximum_copy_length)
{
    if (!destination || !source)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (destination_size == 0)
        return (-1);
    ft_size_t source_length = ft_strlen_size_t(source);
    if (source_length > maximum_copy_length)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (source_length + 1 > destination_size)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    ft_size_t index = 0;
    while (index < source_length)
    {
        destination[index] = source[index];
        ++index;
    }
    destination[index] = '\0';
    return (0);
}
