#include "geometry.hpp"
#include "../Errno/errno.hpp"

bool    intersect_aabb(const aabb &first, const aabb &second)
{
    bool result;

    result = false;
    {
        const aabb *lower;
        const aabb *upper;
        int lock_error;
        bool separated;

        lock_error = first.lock_pair(second, lower, upper);
        if (lock_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(lock_error);
            ft_global_error_stack_push(lock_error);
            ft_global_error_stack_push(lock_error);
            return (false);
        }
        separated = false;
        if (first._maximum_x < second._minimum_x)
            separated = true;
        else if (first._minimum_x > second._maximum_x)
            separated = true;
        else if (first._maximum_y < second._minimum_y)
            separated = true;
        else if (first._minimum_y > second._maximum_y)
            separated = true;
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        first.unlock_pair(lower, upper);
        result = true;
        if (separated)
            result = false;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    if (result)
        return (true);
    return (false);
}

bool    intersect_circle(const circle &first, const circle &second)
{
    bool result;
    const circle *lower;
    const circle *upper;
    int lock_error;
    double  delta_x;
    double  delta_y;
    double  radius_sum;
    double  distance_squared;

    result = false;
    lock_error = first.lock_pair(second, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    delta_x = first._center_x - second._center_x;
    delta_y = first._center_y - second._center_y;
    radius_sum = first._radius + second._radius;
    distance_squared = delta_x * delta_x + delta_y * delta_y;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    first.unlock_pair(lower, upper);
    result = true;
    if (distance_squared > radius_sum * radius_sum)
        result = false;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    if (result)
        return (true);
    return (false);
}

bool    intersect_sphere(const sphere &first, const sphere &second)
{
    bool result;
    const sphere *lower;
    const sphere *upper;
    int lock_error;
    double  delta_x;
    double  delta_y;
    double  delta_z;
    double  radius_sum;
    double  distance_squared;

    result = false;
    lock_error = first.lock_pair(second, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    delta_x = first._center_x - second._center_x;
    delta_y = first._center_y - second._center_y;
    delta_z = first._center_z - second._center_z;
    radius_sum = first._radius + second._radius;
    distance_squared = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    first.unlock_pair(lower, upper);
    result = true;
    if (distance_squared > radius_sum * radius_sum)
        result = false;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    if (result)
        return (true);
    return (false);
}
