#include <cstdlib>

#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int cma_page_prepare_thread_safety(Page *page)
{
    pt_mutex *mutex_pointer;
    void *memory;

    if (!page)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (page->thread_safe_enabled && page->mutex)
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
    page->mutex = mutex_pointer;
    page->thread_safe_enabled = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_page_teardown_thread_safety(Page *page)
{
    if (!page)
        return ;
    if (page->mutex)
    {
        page->mutex->~pt_mutex();
        std::free(page->mutex);
        page->mutex = ft_nullptr;
    }
    page->thread_safe_enabled = false;
    return ;
}

int cma_page_lock(Page *page, bool *lock_acquired)
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!page)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!page->thread_safe_enabled || !page->mutex)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    page->mutex->lock(THREAD_ID);
    if (page->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = page->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_page_unlock(Page *page, bool lock_acquired)
{
    int entry_errno;

    if (!page || !lock_acquired || !page->mutex)
        return ;
    entry_errno = ft_errno;
    page->mutex->unlock(THREAD_ID);
    if (page->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = page->mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

