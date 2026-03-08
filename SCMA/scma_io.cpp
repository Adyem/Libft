#include <cstring>
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

uint32_t    scma_write(scma_handle handle, ft_size_t offset,
                const void *source, ft_size_t size)
{
    scma_block *block;
    unsigned char *heap_data;

    if (scma_mutex_lock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    if (!scma_validate_handle(handle, &block))
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_INVALID_HANDLE)));
    if (!source)
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_INVALID_POINTER)));
    if (offset > block->size)
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_OUT_OF_RANGE)));
    if (size > block->size - offset)
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_OUT_OF_RANGE)));
    heap_data = scma_get_heap_data();
    std::memcpy(heap_data + (block->offset + offset),
        source, size);
    return (static_cast<uint32_t>(scma_unlock_and_return_int(FT_ERR_SUCCESS)));
}

uint32_t    scma_read(scma_handle handle, ft_size_t offset,
                void *destination, ft_size_t size)
{
    scma_block *block;
    unsigned char *heap_data;

    if (scma_mutex_lock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    if (!scma_validate_handle(handle, &block))
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_INVALID_HANDLE)));
    if (!destination)
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_INVALID_POINTER)));
    if (offset > block->size)
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_OUT_OF_RANGE)));
    if (size > block->size - offset)
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_OUT_OF_RANGE)));
    heap_data = scma_get_heap_data();
    std::memcpy(destination, heap_data
            + (block->offset + offset), size);
    return (static_cast<uint32_t>(scma_unlock_and_return_int(FT_ERR_SUCCESS)));
}
