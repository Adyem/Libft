#include "config.hpp"

#if LIBFT_ENABLE_BOUNDS_CHECKED_HELPERS
#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

static void zero_buffer(void *buffer, size_t buffer_size)
{
    if (buffer == ft_nullptr || buffer_size == 0)
        return ;
    unsigned char *bytes = static_cast<unsigned char *>(buffer);
    size_t index = 0;
    while (index < buffer_size)
    {
        bytes[index] = 0;
        ++index;
    }
}

int ft_memcpy_s(void *destination, size_t destination_size, const void *source, size_t number_of_bytes)
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
    unsigned char *destination_bytes = static_cast<unsigned char *>(destination);
    const unsigned char *source_bytes = static_cast<const unsigned char *>(source);
    if ((destination_bytes < source_bytes + number_of_bytes)
        && (source_bytes < destination_bytes + number_of_bytes))
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    ft_memcpy(destination, source, number_of_bytes);
    return (0);
}
#endif
