#include <cstdlib>

#include "logger_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int network_sink_prepare_thread_safety(s_network_sink *sink)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!sink)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (sink->thread_safe_enabled && sink->mutex)
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
    sink->mutex = mutex_pointer;
    sink->thread_safe_enabled = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void network_sink_teardown_thread_safety(s_network_sink *sink)
{
    if (!sink)
        return ;
    if (sink->mutex)
    {
        sink->mutex->~pt_mutex();
        std::free(sink->mutex);
        sink->mutex = ft_nullptr;
    }
    sink->thread_safe_enabled = false;
    return ;
}

int network_sink_lock(const s_network_sink *sink, bool *lock_acquired)
{
    s_network_sink *mutable_sink;

    if (lock_acquired)
        *lock_acquired = false;
    if (!sink)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    mutable_sink = const_cast<s_network_sink *>(sink);
    if (!mutable_sink->thread_safe_enabled || !mutable_sink->mutex)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    mutable_sink->mutex->lock(THREAD_ID);
    if (mutable_sink->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = mutable_sink->mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void network_sink_unlock(const s_network_sink *sink, bool lock_acquired)
{
    s_network_sink *mutable_sink;
    int             entry_errno;

    if (!sink || !lock_acquired)
        return ;
    mutable_sink = const_cast<s_network_sink *>(sink);
    if (!mutable_sink->mutex)
        return ;
    entry_errno = ft_errno;
    mutable_sink->mutex->unlock(THREAD_ID);
    if (mutable_sink->mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = mutable_sink->mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}
