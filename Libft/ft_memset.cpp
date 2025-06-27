#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdint>

void *ft_memset(void *destination, int value, size_t number_of_bytes)
{
    if (destination == ft_nullptr)
        return (ft_nullptr);

    unsigned char *dest = static_cast<unsigned char *>(destination);
    unsigned char byte  = static_cast<unsigned char>(value);

    while (number_of_bytes && (reinterpret_cast<uintptr_t>(dest) & (sizeof(size_t) - 1)))
    {
        *dest++ = byte;
        --number_of_bytes;
    }

    size_t pattern = byte;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    if (sizeof(size_t) == 8)
        pattern |= pattern << 32;

    size_t *dest_word = reinterpret_cast<size_t *>(dest);
    while (number_of_bytes >= sizeof(size_t))
    {
        *dest_word++ = pattern;
        number_of_bytes -= sizeof(size_t);
    }

    dest = reinterpret_cast<unsigned char *>(dest_word);
    while (number_of_bytes)
    {
        *dest++ = byte;
        --number_of_bytes;
    }

    return (destination);
}
