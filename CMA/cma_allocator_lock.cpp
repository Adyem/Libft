#include <cstdlib>
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "cma_internal.hpp"

static bool initialize_cma_allocator_mutex(pt_recursive_mutex **mutex_storage)
{
    void *memory = std::malloc(sizeof(pt_recursive_mutex));
    if (memory == ft_nullptr)
    {
        return (false);
    }
    pt_recursive_mutex *mutex_pointer = new(memory) pt_recursive_mutex();
    *mutex_storage = mutex_pointer;
    return (true);
}

static pt_recursive_mutex *cma_allocator_mutex(void)
{
    static pt_recursive_mutex *mutex_instance = ft_nullptr;
    static bool initialization_attempted = false;
    static bool initialized = false;

    if (!initialization_attempted)
    {
        initialization_attempted = true;
        initialized = initialize_cma_allocator_mutex(&mutex_instance);
        if (!initialized)
            mutex_instance = ft_nullptr;
    }
    if (!initialized)
        return (ft_nullptr);
    return (mutex_instance);
}

int cma_lock_allocator(bool *lock_acquired)
{
    if (!lock_acquired)
        return (FT_ERR_INVALID_ARGUMENT);
    *lock_acquired = false;
    pt_recursive_mutex *mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    int mutex_error = mutex_pointer->lock(THREAD_ID);
    if (mutex_error != 0)
    {
        return (FT_ERR_INVALID_STATE);
    }
    if (cma_metadata_make_writable() != 0)
    {
        mutex_pointer->unlock(THREAD_ID);
        return (FT_ERR_INVALID_STATE);
    }
    bool guard_incremented = cma_metadata_guard_increment();
    if (!guard_incremented)
    {
        mutex_pointer->unlock(THREAD_ID);
        return (FT_ERR_INVALID_STATE);
    }
    *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

int cma_unlock_allocator(bool lock_acquired)
{
    if (!lock_acquired)
        return (FT_ERR_SUCCESSS);
    bool guard_decremented = cma_metadata_guard_decrement();
    pt_recursive_mutex *mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    int mutex_error = mutex_pointer->unlock(THREAD_ID);
    if (!guard_decremented)
        return (FT_ERR_INVALID_STATE);
    if (mutex_error != 0)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESSS);
}
