#include <cstring>
#include "../Errno/errno.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

int32_t    scma_write(scma_handle handle, ft_size_t offset,
            				const void *source, ft_size_t size)
{
    int32_t write_result;
    scma_block *block;
    unsigned char *heap_data;

    write_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_validate_handle(handle, &block))
        return (scma_unlock_and_return_int(0));
    if (!source)
        return (scma_unlock_and_return_int(0));
    if (offset > block->size)
        return (scma_unlock_and_return_int(0));
    if (size > block->size - offset)
        return (scma_unlock_and_return_int(0));
    heap_data = scma_get_heap_data();
    std::memcpy(heap_data + (block->offset + offset),
        source,
        size);
    write_result = 1;
    return (scma_unlock_and_return_int(write_result));
}

int32_t    scma_read(scma_handle handle, ft_size_t offset,
        				void *destination, ft_size_t size)
{
    int32_t read_result;
    scma_block *block;
    unsigned char *heap_data;

    read_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_validate_handle(handle, &block))
        return (scma_unlock_and_return_int(0));
    if (!destination)
        return (scma_unlock_and_return_int(0));
    if (offset > block->size)
        return (scma_unlock_and_return_int(0));
    if (size > block->size - offset)
        return (scma_unlock_and_return_int(0));
    heap_data = scma_get_heap_data();
    std::memcpy(destination, heap_data
			+ (block->offset + offset), size);
    read_result = 1;
    return (scma_unlock_and_return_int(read_result));
}
