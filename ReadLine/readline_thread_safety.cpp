#include "readline_internal.hpp"
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"

int rl_state_prepare_thread_safety(readline_state_t *state)
{
    if (state == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (state->mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

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
    int destroy_error;

    if (state == ft_nullptr)
        return ;
    if (state->mutex == ft_nullptr)
        return ;
    destroy_error = state->mutex->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return ;
    delete state->mutex;
    state->mutex = ft_nullptr;
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
    int mutex_error = state->mutex->lock();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
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
    int mutex_error = state->mutex->unlock();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    return (FT_ERR_SUCCESS);
}
