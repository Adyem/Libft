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
    int error_code;

    write_result = 0;
    if (scma_mutex_lock() != 0)
    {
        error_code = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        scma_record_operation_error(error_code);
        return (0);
    }
    if (!scma_validate_handle(handle, &block))
    {
        error_code = FT_ERR_INVALID_HANDLE;
        scma_record_operation_error(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (!source)
    {
        error_code = FT_ERR_INVALID_POINTER;
        scma_record_operation_error(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (offset > block->size)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        scma_record_operation_error(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (size > block->size - offset)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        scma_record_operation_error(error_code);
        return (scma_unlock_and_return_int(0));
    }
    heap_data = scma_get_heap_data();
    std::memcpy(heap_data + static_cast<size_t>(block->offset + offset),
        source,
        static_cast<size_t>(size));
    error_code = FT_ERR_SUCCESSS;
    scma_record_operation_error(error_code);
    write_result = 1;
    return (scma_unlock_and_return_int(write_result));
}

int    scma_read(scma_handle handle, ft_size_t offset,
            void *destination, ft_size_t size)
{
    int read_result;
    scma_block *block;
    unsigned char *heap_data;
    int error_code;

    read_result = 0;
    if (scma_mutex_lock() != 0)
    {
        error_code = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        scma_record_operation_error(error_code);
        return (0);
    }
    if (!scma_validate_handle(handle, &block))
    {
        error_code = FT_ERR_INVALID_HANDLE;
        scma_record_operation_error(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (!destination)
    {
        error_code = FT_ERR_INVALID_POINTER;
        scma_record_operation_error(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (offset > block->size)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        scma_record_operation_error(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (size > block->size - offset)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        scma_record_operation_error(error_code);
        return (scma_unlock_and_return_int(0));
    }
    heap_data = scma_get_heap_data();
    std::memcpy(destination,
        heap_data + static_cast<size_t>(block->offset + offset),
        static_cast<size_t>(size));
    error_code = FT_ERR_SUCCESSS;
    scma_record_operation_error(error_code);
    read_result = 1;
    return (scma_unlock_and_return_int(read_result));
}
