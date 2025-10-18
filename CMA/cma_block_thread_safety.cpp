#include <cstdlib>

#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int cma_block_prepare_thread_safety(Block *block)
{
    pt_mutex *mutex_pointer;
    void *memory;

    if (!block)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (block->thread_safe_enabled && block->mutex)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
        ft_errno = mutex_error;
        return (-1);
    }
    block->mutex = mutex_pointer;
    block->thread_safe_enabled = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_block_teardown_thread_safety(Block *block)
{
    if (!block)
        return ;
    if (block->mutex)
    {
        block->mutex->~pt_mutex();
        std::free(block->mutex);
        block->mutex = ft_nullptr;
    }
    block->thread_safe_enabled = false;
    return ;
}

int cma_block_lock(Block *block, bool *lock_acquired)
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!block)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!block->thread_safe_enabled || !block->mutex)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    block->mutex->lock(THREAD_ID);
    if (block->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = block->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_block_unlock(Block *block, bool lock_acquired)
{
    int entry_errno;

    if (!block || !lock_acquired || !block->mutex)
        return ;
    entry_errno = ft_errno;
    block->mutex->unlock(THREAD_ID);
    if (block->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = block->mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

