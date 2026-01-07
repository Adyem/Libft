#include "readline_internal.hpp"
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"

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
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (dimensions->thread_safe_enabled == true && dimensions->mutex != ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        delete mutex_pointer;
        ft_errno = mutex_error;
        return (-1);
    }
    dimensions->mutex = mutex_pointer;
    dimensions->thread_safe_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
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
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (dimensions->thread_safe_enabled == false || dimensions->mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    dimensions->mutex->lock(THREAD_ID);
    if (dimensions->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = dimensions->mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void rl_terminal_dimensions_unlock(terminal_dimensions *dimensions, bool lock_acquired)
{
    if (dimensions == ft_nullptr || lock_acquired == false)
        return ;
    if (dimensions->mutex == ft_nullptr)
        return ;
    dimensions->mutex->unlock(THREAD_ID);
    if (dimensions->mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = dimensions->mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int rl_terminal_dimensions_refresh(terminal_dimensions *dimensions)
{
    unsigned short rows;
    unsigned short cols;
    unsigned short x_pixels;
    unsigned short y_pixels;
    bool           lock_acquired;

    if (dimensions == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (rl_terminal_dimensions_lock(dimensions, &lock_acquired) != 0)
        return (-1);
    if (cmp_readline_terminal_dimensions(&rows, &cols, &x_pixels, &y_pixels) != 0)
    {
        rl_terminal_dimensions_clear(dimensions);
        rl_terminal_dimensions_unlock(dimensions, lock_acquired);
        if (ft_errno == FT_ERR_SUCCESSS)
            ft_errno = FT_ERR_TERMINATED;
        return (-1);
    }
    dimensions->rows = rows;
    dimensions->cols = cols;
    dimensions->x_pixels = x_pixels;
    dimensions->y_pixels = y_pixels;
    dimensions->dimensions_valid = true;
    ft_errno = FT_ERR_SUCCESSS;
    rl_terminal_dimensions_unlock(dimensions, lock_acquired);
    if (lock_acquired == true && ft_errno != FT_ERR_SUCCESSS)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

int rl_terminal_dimensions_get(terminal_dimensions *dimensions,
    unsigned short *rows, unsigned short *cols,
    unsigned short *x_pixels, unsigned short *y_pixels,
    bool *dimensions_valid)
{
    bool lock_acquired;

    if (dimensions == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    if (rl_terminal_dimensions_lock(dimensions, &lock_acquired) != 0)
        return (-1);
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
    ft_errno = FT_ERR_SUCCESSS;
    rl_terminal_dimensions_unlock(dimensions, lock_acquired);
    if (lock_acquired == true && ft_errno != FT_ERR_SUCCESSS)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}
