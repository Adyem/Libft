#include "geometry.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

ft_bool    intersect_aabb(const aabb &first, const aabb &second)
{
    ft_bool result;

    result = FT_FALSE;
    {
        const aabb *lower;
        const aabb *upper;
        uint32_t lock_error;
        ft_bool separated;

        lock_error = first.lock_pair(second, lower, upper);
        if (lock_error != FT_ERR_SUCCESS)
            return (FT_FALSE);
        separated = FT_FALSE;
        if (first._maximum_x < second._minimum_x)
            separated = FT_TRUE;
        else if (first._minimum_x > second._maximum_x)
            separated = FT_TRUE;
        else if (first._maximum_y < second._minimum_y)
            separated = FT_TRUE;
        else if (first._minimum_y > second._maximum_y)
            separated = FT_TRUE;
        first.unlock_pair(lower, upper);
        result = FT_TRUE;
        if (separated)
            result = FT_FALSE;
    }
    if (result)
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool    intersect_circle(const circle &first, const circle &second)
{
    ft_bool result;
    const circle *lower;
    const circle *upper;
    uint32_t lock_error;
    double  delta_x;
    double  delta_y;
    double  radius_sum;
    double  distance_squared;

    result = FT_FALSE;
    lock_error = first.lock_pair(second, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_FALSE);
    delta_x = first._center_x - second._center_x;
    delta_y = first._center_y - second._center_y;
    radius_sum = first._radius + second._radius;
    distance_squared = delta_x * delta_x + delta_y * delta_y;
    first.unlock_pair(lower, upper);
    result = FT_TRUE;
    if (distance_squared > radius_sum * radius_sum)
        result = FT_FALSE;
    if (result)
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool    intersect_sphere(const sphere &first, const sphere &second)
{
    ft_bool result;
    const sphere *lower;
    const sphere *upper;
    uint32_t lock_error;
    double  delta_x;
    double  delta_y;
    double  delta_z;
    double  radius_sum;
    double  distance_squared;

    result = FT_FALSE;
    lock_error = first.lock_pair(second, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_FALSE);
    delta_x = first._center_x - second._center_x;
    delta_y = first._center_y - second._center_y;
    delta_z = first._center_z - second._center_z;
    radius_sum = first._radius + second._radius;
    distance_squared = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;
    first.unlock_pair(lower, upper);
    result = FT_TRUE;
    if (distance_squared > radius_sum * radius_sum)
        result = FT_FALSE;
    if (result)
        return (FT_TRUE);
    return (FT_FALSE);
}
