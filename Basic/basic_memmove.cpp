#include "basic.hpp"

void *ft_memmove(void *destination, const void *source, size_t size)
{
    unsigned char *destination_bytes = static_cast<unsigned char *>(destination);
    const unsigned char *source_bytes = static_cast<const unsigned char *>(source);

    if (destination == source || size == 0)
        return (destination);

    if (destination_bytes < source_bytes)
    {
        size_t index = 0;
        while (index < size)
        {
            destination_bytes[index] = source_bytes[index];
            ++index;
        }
    }
    else
    {
        size_t index = size;
        while (index > 0)
        {
            destination_bytes[index - 1] = source_bytes[index - 1];
            --index;
        }
    }

    return (destination);
}
