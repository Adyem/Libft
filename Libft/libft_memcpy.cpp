#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdint>

void* ft_memcpy(void* destination, const void* source, size_t size)
{
    ft_errno = FT_ER_SUCCESSS;
    if (size == 0)
        return (destination);
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }

    if (destination == source)
        return (destination);

    unsigned char*       dest = static_cast<unsigned char*>(destination);
    const unsigned char* src = static_cast<const unsigned char*>(source);
    uintptr_t            dest_address = reinterpret_cast<uintptr_t>(dest);
    uintptr_t            src_address = reinterpret_cast<uintptr_t>(src);
    uintptr_t            range_size;
    uintptr_t            dest_limit;
    uintptr_t            src_limit;

    if (size > UINTPTR_MAX)
        range_size = UINTPTR_MAX;
    else
        range_size = size;
    dest_limit = dest_address + range_size;
    if (dest_limit < dest_address)
        dest_limit = UINTPTR_MAX;
    src_limit = src_address + range_size;
    if (src_limit < src_address)
        src_limit = UINTPTR_MAX;
    if (dest_address < src_limit && src_address < dest_limit)
    {
        ft_errno = FT_ERR_OVERLAP;
        return (destination);
    }

    while (size != 0)
    {
        *dest = *src;
        ++dest;
        ++src;
        --size;
    }

    return (destination);
}
