#include "readline_internal.hpp"
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"

#include "../Errno/errno.hpp"

static int rl_terminal_dimensions_mutex_constructor_error(pt_mutex *mutex_pointer)
{
    int mutex_error;

    mutex_error = mutex_pointer->operation_error_last_error();
    ft_global_error_stack_pop_newest();
    return (mutex_error);
}

static int rl_terminal_dimensions_lock_mutex(pt_mutex *mutex_pointer)
{
    int lock_error;

    lock_error = mutex_pointer->lock(THREAD_ID);
    lock_error = ft_global_error_stack_pop_newest();
    return (lock_error);
}

static int rl_terminal_dimensions_unlock_mutex(pt_mutex *mutex_pointer)
{
    int unlock_error;

    unlock_error = mutex_pointer->unlock(THREAD_ID);
    unlock_error = ft_global_error_stack_pop_newest();
    return (unlock_error);
}

static void rl_terminal_dimensions_clear(terminal_dimensions *dimensions)
{
    if (dimensions == ft_nullptr)
        return ;
    dimensions->rows = 0;
    dimensions->cols = 0;
    dimensions->x_pixels = 0;
    dimensions->y_pixels = 0;
    dimensions->dimensions_valid = false;
    return ;
}

int rl_terminal_dimensions_prepare_thread_safety(terminal_dimensions *dimensions)
{
    pt_mutex *mutex_pointer;

    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (dimensions->thread_safe_enabled == true && dimensions->mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int mutex_error = rl_terminal_dimensions_mutex_constructor_error(mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    dimensions->mutex = mutex_pointer;
    dimensions->thread_safe_enabled = true;
    return (FT_ERR_SUCCESSS);
}

void rl_terminal_dimensions_teardown_thread_safety(terminal_dimensions *dimensions)
{
    if (dimensions == ft_nullptr)
        return ;
    if (dimensions->mutex != ft_nullptr)
    {
        delete dimensions->mutex;
        dimensions->mutex = ft_nullptr;
    }
    dimensions->thread_safe_enabled = false;
    return ;
}

int rl_terminal_dimensions_lock(terminal_dimensions *dimensions, bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (dimensions->thread_safe_enabled == false || dimensions->mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int mutex_error = rl_terminal_dimensions_lock_mutex(dimensions->mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

int rl_terminal_dimensions_unlock(terminal_dimensions *dimensions, bool lock_acquired)
{
    if (dimensions == ft_nullptr || lock_acquired == false)
        return (FT_ERR_INVALID_ARGUMENT);
    if (dimensions->mutex == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    int mutex_error = rl_terminal_dimensions_unlock_mutex(dimensions->mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    return (FT_ERR_SUCCESSS);
}

int rl_terminal_dimensions_refresh(terminal_dimensions *dimensions)
{
    unsigned short rows;
    unsigned short cols;
    unsigned short x_pixels;
    unsigned short y_pixels;
    bool           lock_acquired;
    int            result;

    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = false;
    result = rl_terminal_dimensions_lock(dimensions, &lock_acquired);
    if (result != FT_ERR_SUCCESSS)
        return (result);
    if (cmp_readline_terminal_dimensions(&rows, &cols, &x_pixels, &y_pixels) != 0)
    {
        rl_terminal_dimensions_clear(dimensions);
        result = FT_ERR_TERMINATED;
        goto cleanup;
    }
    dimensions->rows = rows;
    dimensions->cols = cols;
    dimensions->x_pixels = x_pixels;
    dimensions->y_pixels = y_pixels;
    dimensions->dimensions_valid = true;
cleanup:
    if (lock_acquired == true)
    {
        int unlock_error = rl_terminal_dimensions_unlock(dimensions, lock_acquired);

        if (unlock_error != FT_ERR_SUCCESSS && result == FT_ERR_SUCCESSS)
            result = unlock_error;
    }
    return (result);
}

int rl_terminal_dimensions_get(terminal_dimensions *dimensions,
    unsigned short *rows, unsigned short *cols,
    unsigned short *x_pixels, unsigned short *y_pixels,
    bool *dimensions_valid)
{
    bool lock_acquired;
    int  result;

    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = false;
    result = rl_terminal_dimensions_lock(dimensions, &lock_acquired);
    if (result != FT_ERR_SUCCESSS)
        return (result);
    if (rows != ft_nullptr)
        *rows = dimensions->rows;
    if (cols != ft_nullptr)
        *cols = dimensions->cols;
    if (x_pixels != ft_nullptr)
        *x_pixels = dimensions->x_pixels;
    if (y_pixels != ft_nullptr)
        *y_pixels = dimensions->y_pixels;
    if (dimensions_valid != ft_nullptr)
        *dimensions_valid = dimensions->dimensions_valid;
    if (lock_acquired == true)
    {
        int unlock_error = rl_terminal_dimensions_unlock(dimensions, lock_acquired);

        if (unlock_error != FT_ERR_SUCCESSS && result == FT_ERR_SUCCESSS)
            result = unlock_error;
    }
    return (result);
}
