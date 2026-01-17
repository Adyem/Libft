#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdint>

void* ft_memcpy(void* destination, const void* source, size_t size)
{
    if (size == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (destination);
    }
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }

    if (destination == source)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (destination);
    }

    unsigned char*       destination_bytes = static_cast<unsigned char*>(destination);
    const unsigned char* source_bytes = static_cast<const unsigned char*>(source);
    uintptr_t            destination_address = reinterpret_cast<uintptr_t>(destination_bytes);
    uintptr_t            source_address = reinterpret_cast<uintptr_t>(source_bytes);
    uintptr_t            range_size;
    uintptr_t            dest_limit;
    uintptr_t            src_limit;

    if (size > UINTPTR_MAX)
        range_size = UINTPTR_MAX;
    else
        range_size = size;
    dest_limit = destination_address + range_size;
    if (dest_limit < destination_address)
        dest_limit = UINTPTR_MAX;
    src_limit = source_address + range_size;
    if (src_limit < source_address)
        src_limit = UINTPTR_MAX;
    if (destination_address < src_limit && source_address < dest_limit)
    {
        ft_global_error_stack_push(FT_ERR_OVERLAP);
        return (destination);
    }

    while (size != 0)
    {
        *destination_bytes = *source_bytes;
        ++destination_bytes;
        ++source_bytes;
        --size;
    }

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (destination);
}
