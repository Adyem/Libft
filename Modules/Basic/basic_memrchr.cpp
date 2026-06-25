#include "basic.hpp"
#include "../Basic/class_nullptr.hpp"

void *ft_memrchr(const void *pointer, int32_t number, ft_size_t size)
{
    const unsigned char *bytes;
    unsigned char character;

    if (size == 0 || pointer == ft_nullptr)
        return (ft_nullptr);
    bytes = static_cast<const unsigned char *>(pointer);
    character = static_cast<unsigned char>(number);
    while (size > 0)
    {
        --size;
        if (bytes[size] == character)
            return (const_cast<void *>(static_cast<const void *>(&bytes[size])));
    }
    return (ft_nullptr);
}
