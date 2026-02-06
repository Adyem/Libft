#include "recursive_mutex.hpp"
#include "pthread.hpp"
#include "pthread_internal.hpp"
#include "mutex.hpp"
#include <pthread.h>
#include <new>
#include "../Errno/errno.hpp"

int pt_recursive_mutex_lock_with_error(const pt_recursive_mutex &mutex)
{
    int mutex_error;
    int stack_error;

    mutex_error = mutex.lock(THREAD_ID);
    stack_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(stack_error);
        return (mutex_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int pt_recursive_mutex_unlock_with_error(const pt_recursive_mutex &mutex)
{
    int mutex_error;
    int stack_error;

    mutex_error = mutex.unlock(THREAD_ID);
    stack_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(stack_error);
        return (mutex_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int pt_mutex_lock_with_error(const pt_mutex &mutex)
{
    int mutex_error;
    int stack_error;

    mutex_error = mutex.lock(THREAD_ID);
    stack_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(stack_error);
        return (mutex_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int pt_mutex_unlock_with_error(const pt_mutex &mutex)
{
    int mutex_error;
    int stack_error;

    mutex_error = mutex.unlock(THREAD_ID);
    stack_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(stack_error);
        return (mutex_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

int pt_recursive_mutex_lock_if_valid(pt_recursive_mutex *mutex)
{
    if (mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_lock_with_error(*mutex));
}

int pt_recursive_mutex_unlock_if_valid(pt_recursive_mutex *mutex)
{
    if (mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*mutex));
}

int pt_recursive_mutex_lock_if_enabled(pt_recursive_mutex *mutex, bool thread_safe_enabled)
{
    if (!thread_safe_enabled)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_lock_if_valid(mutex));
}

int pt_recursive_mutex_unlock_if_enabled(pt_recursive_mutex *mutex, bool thread_safe_enabled)
{
    if (!thread_safe_enabled)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_if_valid(mutex));
}

int pt_mutex_lock_if_valid(pt_mutex *mutex)
{
    if (mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_mutex_lock_with_error(*mutex));
}

int pt_mutex_unlock_if_valid(pt_mutex *mutex)
{
    if (mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_mutex_unlock_with_error(*mutex));
}

int pt_recursive_mutex_create_with_error(pt_recursive_mutex **mutex)
{
    if (mutex == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    pt_recursive_mutex *mutex_instance = new (std::nothrow) pt_recursive_mutex();
    if (mutex_instance == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int creation_error = ft_global_error_stack_drop_last_error();
    if (creation_error != FT_ERR_SUCCESSS)
    {
        delete mutex_instance;
        return (creation_error);
    }
    *mutex = mutex_instance;
    return (FT_ERR_SUCCESSS);
}

void pt_recursive_mutex_destroy(pt_recursive_mutex **mutex)
{
    if (mutex == ft_nullptr || *mutex == ft_nullptr)
        return ;
    delete *mutex;
    *mutex = ft_nullptr;
    return ;
}

int pt_mutex_create_with_error(pt_mutex **mutex)
{
    if (mutex == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    pt_mutex *mutex_instance = new (std::nothrow) pt_mutex();
    if (mutex_instance == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int creation_error = ft_global_error_stack_drop_last_error();
    if (creation_error != FT_ERR_SUCCESSS)
    {
        delete mutex_instance;
        return (creation_error);
    }
    *mutex = mutex_instance;
    return (FT_ERR_SUCCESSS);
}

void pt_mutex_destroy(pt_mutex **mutex)
{
    if (mutex == ft_nullptr || *mutex == ft_nullptr)
        return ;
    delete *mutex;
    *mutex = ft_nullptr;
    return ;
}

int pt_pthread_mutex_lock_with_error(pthread_mutex_t *mutex)
{
    int mutex_result;
    int error_code;

    mutex_result = pthread_mutex_lock(mutex);
    if (mutex_result != 0)
        error_code = ft_map_system_error(mutex_result);
    else
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (error_code);
}

int pt_pthread_mutex_unlock_with_error(pthread_mutex_t *mutex)
{
    int mutex_result;
    int error_code;

    mutex_result = pthread_mutex_unlock(mutex);
    if (mutex_result != 0)
        error_code = ft_map_system_error(mutex_result);
    else
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (error_code);
}
