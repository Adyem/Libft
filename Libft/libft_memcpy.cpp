#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdint>

void* ft_memcpy(void* destination, const void* source, size_t size)
{
    ft_errno = ER_SUCCESS;
    if (size == 0)
        return (destination);
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }

    unsigned char*       dest = static_cast<unsigned char*>(destination);
    const unsigned char* src = static_cast<const unsigned char*>(source);
    const unsigned char* dest_end;
    const unsigned char* src_end;
    uintptr_t            alignment_mask = sizeof(size_t) - 1;

    dest_end = dest + size;
    src_end = src + size;
    if (dest < src_end && src < dest_end)
    {
        ft_errno = FT_EOVERLAP;
        return (destination);
    }

    if ((reinterpret_cast<uintptr_t>(dest) & alignment_mask) != (reinterpret_cast<uintptr_t>(src) & alignment_mask))
    {
        while (size)
        {
            *dest++ = *src++;
            --size;
        }
        return (destination);
    }

    while (size && (reinterpret_cast<uintptr_t>(dest) & alignment_mask))
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
