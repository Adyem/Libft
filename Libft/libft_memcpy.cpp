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
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }

    unsigned char*       dest = static_cast<unsigned char*>(destination);
    const unsigned char* src = static_cast<const unsigned char*>(source);
    uintptr_t            dest_address = reinterpret_cast<uintptr_t>(dest);
    uintptr_t            src_address = reinterpret_cast<uintptr_t>(src);
    uintptr_t            range_size;
    uintptr_t            dest_end_address;
    uintptr_t            src_end_address;

    if (size > UINTPTR_MAX)
        range_size = UINTPTR_MAX;
    else
        range_size = static_cast<uintptr_t>(size);
    if (UINTPTR_MAX - dest_address < range_size)
        dest_end_address = UINTPTR_MAX;
    else
        dest_end_address = dest_address + range_size;
    if (UINTPTR_MAX - src_address < range_size)
        src_end_address = UINTPTR_MAX;
    else
        src_end_address = src_address + range_size;
    if (dest_address != src_address && dest_address < src_end_address && src_address < dest_end_address)
    {
        ft_errno = FT_ERR_OVERLAP;
        return (destination);
    }

    while (size)
    {
        *dest++ = *src++;
        --size;
    }

    return (destination);
}
