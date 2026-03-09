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
    dimensions->row_count = 0;
    dimensions->column_count = 0;
    dimensions->x_pixels = 0;
    dimensions->y_pixels = 0;
    dimensions->dimensions_valid = FT_FALSE;
    return ;
}
int32_t rl_terminal_dimensions_prepare_thread_safety(terminal_dimensions *dimensions)
{
    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (dimensions->mutex != ft_nullptr)
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
    dimensions->mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void rl_terminal_dimensions_teardown_thread_safety(terminal_dimensions *dimensions)
{
    if (dimensions == ft_nullptr)
        return ;
    if (dimensions->mutex == ft_nullptr)
        return ;
    (void)dimensions->mutex->destroy();
    delete dimensions->mutex;
    dimensions->mutex = ft_nullptr;
    return ;
}

int32_t rl_terminal_dimensions_lock(terminal_dimensions *dimensions, ft_bool *lock_acquired)
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    int32_t mutex_error = pt_recursive_mutex_lock_if_not_null(dimensions->mutex);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr && dimensions->mutex != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

int32_t rl_terminal_dimensions_unlock(terminal_dimensions *dimensions, ft_bool lock_acquired)
{
    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(dimensions->mutex);
    return (FT_ERR_SUCCESS);
}

int32_t rl_terminal_dimensions_refresh(terminal_dimensions *dimensions)
{
    uint16_t row_count;
    uint16_t column_count;
    uint16_t x_pixels;
    uint16_t y_pixels;
    ft_bool           lock_acquired;
    int32_t            result;

    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    result = rl_terminal_dimensions_lock(dimensions, &lock_acquired);
    if (result != FT_ERR_SUCCESS)
        return (result);
    if (cmp_readline_terminal_dimensions(&row_count, &column_count, &x_pixels, &y_pixels) != FT_ERR_SUCCESS)
    {
        rl_terminal_dimensions_clear(dimensions);
        result = FT_ERR_TERMINATED;
        goto cleanup;
    }
    dimensions->row_count = row_count;
    dimensions->column_count = column_count;
    dimensions->x_pixels = x_pixels;
    dimensions->y_pixels = y_pixels;
    dimensions->dimensions_valid = FT_TRUE;
cleanup:
    if (lock_acquired == FT_TRUE)
        (void)rl_terminal_dimensions_unlock(dimensions, lock_acquired);
    return (result);
}

int32_t rl_terminal_dimensions_get(terminal_dimensions *dimensions,
    uint16_t *row_count, uint16_t *column_count,
    uint16_t *x_pixels, uint16_t *y_pixels,
    ft_bool *dimensions_valid)
{
    ft_bool lock_acquired;
    int32_t  result;

    if (dimensions == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    result = rl_terminal_dimensions_lock(dimensions, &lock_acquired);
    if (result != FT_ERR_SUCCESS)
        return (result);
    if (row_count != ft_nullptr)
        *row_count = dimensions->row_count;
    if (column_count != ft_nullptr)
        *column_count = dimensions->column_count;
    if (x_pixels != ft_nullptr)
        *x_pixels = dimensions->x_pixels;
    if (y_pixels != ft_nullptr)
        *y_pixels = dimensions->y_pixels;
    if (dimensions_valid != ft_nullptr)
        *dimensions_valid = dimensions->dimensions_valid;
    if (lock_acquired == FT_TRUE)
        (void)rl_terminal_dimensions_unlock(dimensions, lock_acquired);
    return (result);
}
