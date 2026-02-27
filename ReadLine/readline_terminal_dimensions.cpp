#include "readline_internal.hpp"
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"

#include "../Errno/errno.hpp"

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
    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (dimensions->mutex != ft_nullptr)
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
    dimensions->mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void rl_terminal_dimensions_teardown_thread_safety(terminal_dimensions *dimensions)
{
    int destroy_error;

    if (dimensions == ft_nullptr)
        return ;
    if (dimensions->mutex == ft_nullptr)
        return ;
    destroy_error = dimensions->mutex->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return ;
    delete dimensions->mutex;
    dimensions->mutex = ft_nullptr;
    return ;
}

int rl_terminal_dimensions_lock(terminal_dimensions *dimensions, bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    int mutex_error = pt_recursive_mutex_lock_if_not_null(dimensions->mutex);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr && dimensions->mutex != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int rl_terminal_dimensions_unlock(terminal_dimensions *dimensions, bool lock_acquired)
{
    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    int mutex_error = pt_recursive_mutex_unlock_if_not_null(dimensions->mutex);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    return (FT_ERR_SUCCESS);
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
    if (result != FT_ERR_SUCCESS)
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

        if (unlock_error != FT_ERR_SUCCESS && result == FT_ERR_SUCCESS)
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
    if (result != FT_ERR_SUCCESS)
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

        if (unlock_error != FT_ERR_SUCCESS && result == FT_ERR_SUCCESS)
            result = unlock_error;
    }
    return (result);
}
