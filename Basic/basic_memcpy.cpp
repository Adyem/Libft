#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

void* ft_memcpy(void* destination, const void* source, ft_size_t size)
{
    if (size == 0 || destination == ft_nullptr || source == ft_nullptr)
        return (destination);

    unsigned char* destination_bytes = static_cast<unsigned char*>(destination);
    const unsigned char* source_bytes = static_cast<const unsigned char*>(source);

    ft_size_t index = 0;
    while (index < size)
    {
        destination_bytes[index] = source_bytes[index];
        ++index;
    }

    return (destination);
}
