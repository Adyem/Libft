#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

void* ft_memchr(const void* pointer, int32_t number, ft_size_t size)
{
    const unsigned char *bytes;
    unsigned char character;

    if (size == 0 || pointer == ft_nullptr)
        return (ft_nullptr);
    bytes = static_cast<const unsigned char*>(pointer);
    character = static_cast<unsigned char>(number);
    ft_size_t index = 0;
    while (index < size)
    {
        if (bytes[index] == character)
            return (const_cast<void*>(static_cast<const void*>(&bytes[index])));
        ++index;
    }
    return (ft_nullptr);
}
