#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdint>

void* ft_memchr(const void* pointer, int number, size_t size)
{
    if (!pointer)
        return (ft_nullptr);
    const unsigned char* p = static_cast<const unsigned char*>(pointer);
    unsigned char c = static_cast<unsigned char>(number);
    while (size && (reinterpret_cast<uintptr_t>(p) % sizeof(size_t)))
    {
        if (*p == c)
            return (const_cast<unsigned char*>(p));
        ++p;
        --size;
    }
    const size_t ones = ~static_cast<size_t>(0) / 0xFF;
    const size_t high_bits = ones * 0x80;
    const size_t char_mask = ones * c;
    const size_t* lp = reinterpret_cast<const size_t*>(p);
    while (size >= sizeof(size_t))
    {
        size_t x = *lp ^ char_mask;
        if (((x - ones) & ~x & high_bits) != 0)
        {
            p = reinterpret_cast<const unsigned char*>(lp);
            for (size_t i = 0; i < sizeof(size_t); ++i)
            {
                if (p[i] == c)
                    return (const_cast<unsigned char*>(p + i));
            }
        }
        ++lp;
        size -= sizeof(size_t);
    }
    p = reinterpret_cast<const unsigned char*>(lp);
    while (size--)
    {
        if (*p == c)
            return (const_cast<unsigned char*>(p));
        ++p;
    }
    return (ft_nullptr);
}
