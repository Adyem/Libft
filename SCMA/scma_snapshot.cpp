#include <cstdlib>
#include <cstring>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

void    *scma_snapshot(scma_handle handle, ft_size_t *size)
{
    void *copy;
    void *snapshot_result;
    scma_block *block;
    unsigned char *heap_data;

    copy = ft_nullptr;
    snapshot_result = ft_nullptr;
    if (scma_mutex_lock() != 0)
        return (ft_nullptr);
    if (!scma_validate_handle(handle, &block))
        return (scma_unlock_and_return_pointer(ft_nullptr));
    if (size)
        *size = block->size;
    if (block->size == 0)
    {
        scma_reset_live_snapshot();
        return (scma_unlock_and_return_pointer(ft_nullptr));
    }
    copy = std::malloc(static_cast<size_t>(block->size));
    if (!copy)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        scma_reset_live_snapshot();
        return (scma_unlock_and_return_pointer(ft_nullptr));
    }
    heap_data = scma_get_heap_data();
    std::memcpy(copy,
        heap_data + static_cast<size_t>(block->offset),
        static_cast<size_t>(block->size));
    if (size)
        scma_track_live_snapshot(handle, static_cast<unsigned char *>(copy), block->size, 1);
    else
        scma_track_live_snapshot(scma_invalid_handle(), ft_nullptr, 0, 0);
    ft_errno = FT_ERR_SUCCESSS;
    snapshot_result = copy;
    copy = ft_nullptr;
    if (copy)
        std::free(copy);
    if (scma_mutex_unlock() != 0)
    {
        if (snapshot_result)
        {
            std::free(snapshot_result);
            snapshot_result = ft_nullptr;
        }
        return (ft_nullptr);
    }
    return (snapshot_result);
}

int    scma_release_snapshot(void *snapshot_buffer)
{
    int release_result;
    scma_live_snapshot &snapshot = scma_live_snapshot_ref();

    release_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_initialized_ref())
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (scma_unlock_and_return_int(0));
    }
    if (!snapshot.active)
    {
        if (snapshot_buffer != ft_nullptr)
        {
            ft_errno = FT_ERR_INVALID_POINTER;
            return (scma_unlock_and_return_int(0));
        }
        scma_reset_live_snapshot();
        ft_errno = FT_ERR_SUCCESSS;
        release_result = 1;
        return (scma_unlock_and_return_int(release_result));
    }
    if (snapshot_buffer != snapshot.data)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (scma_unlock_and_return_int(0));
    }
    scma_reset_live_snapshot();
    ft_errno = FT_ERR_SUCCESSS;
    release_result = 1;
    return (scma_unlock_and_return_int(release_result));
}
