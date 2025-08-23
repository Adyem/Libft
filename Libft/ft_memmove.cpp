#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdint>

void *ft_memmove(void *destination, const void *source, size_t size)
{
    if (!destination || !source)
        return (ft_nullptr);
    unsigned char *dest_ptr = static_cast<unsigned char*>(destination);
    const unsigned char *src_ptr = static_cast<const unsigned char*>(source);
    if (dest_ptr == src_ptr || size == 0)
        return (destination);
    if (dest_ptr < src_ptr)
    {
        size_t i = 0;
        while (i + sizeof(size_t) <= size)
        {
            *reinterpret_cast<size_t*>(dest_ptr + i) =
                *reinterpret_cast<const size_t*>(src_ptr + i);
            i += sizeof(size_t);
        }
        while (i < size)
        {
            dest_ptr[i] = src_ptr[i];
            ++i;
        }
    }
    else
    {
        size_t i = size;
        while (i >= sizeof(size_t))
        {
            i -= sizeof(size_t);
            *reinterpret_cast<size_t*>(dest_ptr + i) =
                *reinterpret_cast<const size_t*>(src_ptr + i);
        }
        while (i-- > 0)
            dest_ptr[i] = src_ptr[i];
    }
    return (destination);
}
