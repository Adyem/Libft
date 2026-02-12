#include "readline_internal.hpp"
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"

int rl_state_prepare_thread_safety(readline_state_t *state)
{

    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (state->mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    state->mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void rl_state_teardown_thread_safety(readline_state_t *state)
{
    if (state == ft_nullptr)
        return ;
    pt_recursive_mutex_destroy(&state->mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

int rl_state_lock(readline_state_t *state, bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (state->mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int mutex_error = pt_recursive_mutex_lock_with_error(*state->mutex);
    int stack_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(stack_error);
        return (mutex_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int rl_state_unlock(readline_state_t *state, bool lock_acquired)
{
    if (state == ft_nullptr || lock_acquired == false)
        return (FT_ERR_INVALID_ARGUMENT);
    if (state->mutex == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    int mutex_error = pt_recursive_mutex_unlock_with_error(*state->mutex);
    int stack_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(stack_error);
        return (mutex_error);
    }
    return (FT_ERR_SUCCESS);
}
