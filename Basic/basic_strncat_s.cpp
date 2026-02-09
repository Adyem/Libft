#include <cstddef>
#include "basic.hpp"

static void zero_buffer(char *buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0)
        return ;
    size_t index = 0;
    while (index < buffer_size)
    {
        buffer[index] = '\0';
        ++index;
    }
}

int ft_strncat_s(char *destination, size_t destination_size,
                    const char *source, size_t maximum_append_length)
{
    if (!destination || !source)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (destination_size == 0)
        return (-1);
    size_t destination_length = ft_strlen_size_t(destination);
    size_t source_length = ft_strlen_size_t(source);
    if (destination_length >= destination_size)
        return (-1);
    if (source_length > maximum_append_length)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (destination_length + source_length + 1 > destination_size)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    size_t index = 0;
    while (index < source_length)
    {
        destination[destination_length + index] = source[index];
        ++index;
    }
    destination[destination_length + index] = '\0';
    return (0);
}
