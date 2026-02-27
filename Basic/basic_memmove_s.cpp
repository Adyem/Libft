#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

static void zero_buffer(void *buffer, ft_size_t buffer_size)
{
    if (buffer == ft_nullptr || buffer_size == 0)
        return ;

    unsigned char *bytes = static_cast<unsigned char *>(buffer);
    ft_size_t index = 0;
    while (index < buffer_size)
    {
        bytes[index] = 0;
        ++index;
    }
    return ;
}

int32_t ft_memmove_s(void *destination, ft_size_t destination_size, const void *source,
        ft_size_t number_of_bytes)
{
    if (number_of_bytes == 0)
        return (0);
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (destination_size < number_of_bytes)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if ((static_cast<unsigned char *>(destination)
                < static_cast<const unsigned char *>(source) + number_of_bytes)
        && (static_cast<const unsigned char *>(source)
            < static_cast<unsigned char *>(destination) + number_of_bytes))
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    ft_memmove(destination, source, number_of_bytes);
    return (0);
}
