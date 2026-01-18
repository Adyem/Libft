#include "readline_internal.hpp"
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int rl_state_prepare_thread_safety(readline_state_t *state)
{
    pt_mutex *mutex_pointer;
    int error_code;

    if (state == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (state->thread_safe_enabled == true && state->mutex != ft_nullptr)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        delete mutex_pointer;
        ft_global_error_stack_push(mutex_error);
        return (-1);
    }
    state->mutex = mutex_pointer;
    state->thread_safe_enabled = true;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}

void rl_state_teardown_thread_safety(readline_state_t *state)
{
    if (state == ft_nullptr)
        return ;
    if (state->mutex != ft_nullptr)
    {
        delete state->mutex;
        state->mutex = ft_nullptr;
    }
    state->thread_safe_enabled = false;
    return ;
}

int rl_state_lock(readline_state_t *state, bool *lock_acquired)
{
    int error_code;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (state == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (state->thread_safe_enabled == false || state->mutex == ft_nullptr)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    state->mutex->lock(THREAD_ID);
    if (state->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        error_code = state->mutex->get_error();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}

void rl_state_unlock(readline_state_t *state, bool lock_acquired)
{
    int error_code;

    if (state == ft_nullptr || lock_acquired == false)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return ;
    }
    if (state->mutex == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_STATE;
        ft_global_error_stack_push(error_code);
        return ;
    }
    state->mutex->unlock(THREAD_ID);
    if (state->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        error_code = state->mutex->get_error();
        ft_global_error_stack_push(error_code);
        return ;
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return ;
}
