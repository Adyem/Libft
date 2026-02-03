#include "readline_internal.hpp"
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

static int rl_state_lock_mutex(pt_mutex *mutex_pointer)
{
    int lock_error;

    lock_error = mutex_pointer->lock(THREAD_ID);
    lock_error = ft_global_error_stack_pop_newest();
    return (lock_error);
}

static int rl_state_unlock_mutex(pt_mutex *mutex_pointer)
{
    int unlock_error;

    unlock_error = mutex_pointer->unlock(THREAD_ID);
    unlock_error = ft_global_error_stack_pop_newest();
    return (unlock_error);
}

int rl_state_prepare_thread_safety(readline_state_t *state)
{
    pt_mutex *mutex_pointer;

    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (state->thread_safe_enabled == true && state->mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int mutex_error;

    mutex_error = ft_global_error_stack_last_error();
    ft_global_error_stack_pop_newest();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    state->mutex = mutex_pointer;
    state->thread_safe_enabled = true;
    return (FT_ERR_SUCCESSS);
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
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (state->thread_safe_enabled == false || state->mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int mutex_error = rl_state_lock_mutex(state->mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

int rl_state_unlock(readline_state_t *state, bool lock_acquired)
{
    if (state == ft_nullptr || lock_acquired == false)
        return (FT_ERR_INVALID_ARGUMENT);
    if (state->mutex == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    int mutex_error = rl_state_unlock_mutex(state->mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    return (FT_ERR_SUCCESSS);
}
