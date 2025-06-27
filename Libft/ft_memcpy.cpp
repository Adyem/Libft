#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdint>

void* ft_memcpy(void* destination, const void* source, size_t size)
{
    if (destination == ft_nullptr || source == ft_nullptr)
        return (ft_nullptr);

    unsigned char*       dest = static_cast<unsigned char*>(destination);
    const unsigned char* src = static_cast<const unsigned char*>(source);

    while (size && (reinterpret_cast<uintptr_t>(dest) & (sizeof(size_t) - 1)))
    {
        *dest++ = *src++;
        --size;
    }

    size_t*       dest_word = reinterpret_cast<size_t*>(dest);
    const size_t* src_word  = reinterpret_cast<const size_t*>(src);

    while (size >= sizeof(size_t))
    {
        *dest_word++ = *src_word++;
        size -= sizeof(size_t);
    }

    dest = reinterpret_cast<unsigned char*>(dest_word);
    src  = reinterpret_cast<const unsigned char*>(src_word);

    while (size)
    {
        *dest++ = *src++;
        --size;
    }

    return (destination);
}
