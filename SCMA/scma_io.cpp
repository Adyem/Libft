#include <cstring>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

int    scma_write(scma_handle handle, ft_size_t offset,
            const void *source, ft_size_t size)
{
    int write_result;
    scma_block *block;
    unsigned char *heap_data;

    write_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_validate_handle(handle, &block))
        return (scma_unlock_and_return_int(0));
    if (!source)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (scma_unlock_and_return_int(0));
    }
    if (offset > block->size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (scma_unlock_and_return_int(0));
    }
    if (size > block->size - offset)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (scma_unlock_and_return_int(0));
    }
    heap_data = scma_get_heap_data();
    std::memcpy(heap_data + static_cast<size_t>(block->offset + offset),
        source,
        static_cast<size_t>(size));
    scma_update_tracked_snapshot(handle, offset, source, size);
    ft_errno = FT_ERR_SUCCESSS;
    write_result = 1;
    return (scma_unlock_and_return_int(write_result));
}

int    scma_read(scma_handle handle, ft_size_t offset,
            void *destination, ft_size_t size)
{
    int read_result;
    scma_block *block;
    unsigned char *heap_data;

    read_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_validate_handle(handle, &block))
        return (scma_unlock_and_return_int(0));
    if (!destination)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (scma_unlock_and_return_int(0));
    }
    if (offset > block->size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (scma_unlock_and_return_int(0));
    }
    if (size > block->size - offset)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (scma_unlock_and_return_int(0));
    }
    heap_data = scma_get_heap_data();
    std::memcpy(destination,
        heap_data + static_cast<size_t>(block->offset + offset),
        static_cast<size_t>(size));
    ft_errno = FT_ERR_SUCCESSS;
    read_result = 1;
    return (scma_unlock_and_return_int(read_result));
}
