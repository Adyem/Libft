#include <cstdlib>

#include "networking.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

void event_loop_init(event_loop *loop)
{
    if (!loop)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    loop->read_file_descriptors = ft_nullptr;
    loop->read_count = 0;
    loop->write_file_descriptors = ft_nullptr;
    loop->write_count = 0;
    loop->mutex = ft_nullptr;
    loop->thread_safe_enabled = false;
    if (event_loop_prepare_thread_safety(loop) != 0)
        return ;
    return ;
}

void event_loop_clear(event_loop *loop)
{
    bool lock_acquired;
    int  lock_result;

    if (!loop)
        return ;
    lock_acquired = false;
    lock_result = event_loop_lock(loop, &lock_acquired);
    if (loop->read_file_descriptors)
    {
        cma_free(loop->read_file_descriptors);
        loop->read_file_descriptors = ft_nullptr;
    }
    if (loop->write_file_descriptors)
    {
        cma_free(loop->write_file_descriptors);
        loop->write_file_descriptors = ft_nullptr;
    }
    loop->read_count = 0;
    loop->write_count = 0;
    if (lock_result == 0)
        event_loop_unlock(loop, lock_acquired);
    event_loop_teardown_thread_safety(loop);
    return ;
}

int event_loop_add_socket(event_loop *loop, int socket_fd, bool is_write)
{
    int  *new_array;
    int **descriptor_array;
    int  *descriptor_count;
    int   current_count;
    bool  lock_acquired;
    int   result;

    if (!loop)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (event_loop_prepare_thread_safety(loop) != 0)
        return (-1);
    lock_acquired = false;
    if (event_loop_lock(loop, &lock_acquired) != 0)
        return (-1);
    if (is_write)
    {
        descriptor_array = &loop->write_file_descriptors;
        descriptor_count = &loop->write_count;
    }
    else
    {
        descriptor_array = &loop->read_file_descriptors;
        descriptor_count = &loop->read_count;
    }
    new_array = static_cast<int *>(cma_realloc(*descriptor_array,
                                               sizeof(int) * (*descriptor_count + 1)));
    if (!new_array)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        result = -1;
    }
    else
    {
        *descriptor_array = new_array;
        current_count = *descriptor_count;
        new_array[current_count] = socket_fd;
        *descriptor_count = current_count + 1;
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        result = 0;
    }
    event_loop_unlock(loop, lock_acquired);
    return (result);
}

int event_loop_remove_socket(event_loop *loop, int socket_fd, bool is_write)
{
    int  *descriptor_count;
    int  *descriptors;
    int   index;
    bool  lock_acquired;
    int   result;

    if (!loop)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (event_loop_prepare_thread_safety(loop) != 0)
        return (-1);
    lock_acquired = false;
    if (event_loop_lock(loop, &lock_acquired) != 0)
        return (-1);
    if (is_write)
    {
        descriptors = loop->write_file_descriptors;
        descriptor_count = &loop->write_count;
    }
    else
    {
        descriptors = loop->read_file_descriptors;
        descriptor_count = &loop->read_count;
    }
    index = 0;
    while (index < *descriptor_count)
    {
        if (descriptors[index] == socket_fd)
            break;
        index++;
    }
    if (index == *descriptor_count)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        result = -1;
    }
    else
    {
        while (index + 1 < *descriptor_count)
        {
            descriptors[index] = descriptors[index + 1];
            index++;
        }
        *descriptor_count = *descriptor_count - 1;
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        result = 0;
    }
    event_loop_unlock(loop, lock_acquired);
    return (result);
}

int event_loop_run(event_loop *loop, int timeout_milliseconds)
{
    int  poll_result;
    bool lock_acquired;

    if (!loop)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (event_loop_prepare_thread_safety(loop) != 0)
        return (-1);
    lock_acquired = false;
    if (event_loop_lock(loop, &lock_acquired) != 0)
        return (-1);
    poll_result = nw_poll(loop->read_file_descriptors, loop->read_count,
                          loop->write_file_descriptors, loop->write_count,
                          timeout_milliseconds);
    event_loop_unlock(loop, lock_acquired);
    return (poll_result);
}

int event_loop_prepare_thread_safety(event_loop *loop)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (!loop)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (loop->thread_safe_enabled && loop->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = std::malloc(sizeof(pt_mutex));
    if (!memory)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory);
        ft_global_error_stack_push(mutex_error);
        return (-1);
    }
    loop->mutex = mutex_pointer;
    loop->thread_safe_enabled = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void event_loop_teardown_thread_safety(event_loop *loop)
{
    if (!loop)
        return ;
    if (loop->mutex)
    {
        loop->mutex->~pt_mutex();
        std::free(loop->mutex);
        loop->mutex = ft_nullptr;
    }
    loop->thread_safe_enabled = false;
    return ;
}

int event_loop_lock(event_loop *loop, bool *lock_acquired)
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!loop)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!loop->thread_safe_enabled || !loop->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    loop->mutex->lock(THREAD_ID);
    if (loop->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(loop->mutex->get_error());
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void event_loop_unlock(event_loop *loop, bool lock_acquired)
{
    if (!loop)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (!lock_acquired || !loop->mutex)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    loop->mutex->unlock(THREAD_ID);
    if (loop->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(loop->mutex->get_error());
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
