#include "readline_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

int rl_state_prepare_thread_safety(readline_state_t *state)
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (state == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (state->thread_safe_enabled == true && state->mutex != ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        ft_errno = mutex_error;
        return (-1);
    }
    state->mutex = mutex_pointer;
    state->thread_safe_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void rl_state_teardown_thread_safety(readline_state_t *state)
{
    if (state == ft_nullptr)
        return ;
    if (state->mutex != ft_nullptr)
    {
        state->mutex->~pt_mutex();
        cma_free(state->mutex);
        state->mutex = ft_nullptr;
    }
    state->thread_safe_enabled = false;
    return ;
}

int rl_state_lock(readline_state_t *state, bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (state == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (state->thread_safe_enabled == false || state->mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    state->mutex->lock(THREAD_ID);
    if (state->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = state->mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void rl_state_unlock(readline_state_t *state, bool lock_acquired)
{
    if (state == ft_nullptr || lock_acquired == false)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (state->mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return ;
    }
    state->mutex->unlock(THREAD_ID);
    if (state->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = state->mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}
