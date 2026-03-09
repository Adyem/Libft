#include "readline_internal.hpp"
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"

int32_t rl_state_prepare_thread_safety(readline_state_t *state)
{
    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (state->mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    state->mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void rl_state_teardown_thread_safety(readline_state_t *state)
{
    if (state == ft_nullptr)
        return ;
    if (state->mutex == ft_nullptr)
        return ;
    (void)state->mutex->destroy();
    delete state->mutex;
    state->mutex = ft_nullptr;
    return ;
}

int32_t rl_state_lock(readline_state_t *state, ft_bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    int32_t mutex_error = pt_recursive_mutex_lock_if_not_null(state->mutex);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr && state->mutex != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

int32_t rl_state_unlock(readline_state_t *state, ft_bool lock_acquired)
{
    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(state->mutex);
    return (FT_ERR_SUCCESS);
}
