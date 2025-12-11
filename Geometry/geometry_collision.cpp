#include "geometry.hpp"
#include "../Errno/errno.hpp"

#include "../PThread/unique_lock.hpp"

bool    intersect_aabb(const aabb &first, const aabb &second)
{
    bool result;

    result = false;
    {
        ft_unique_lock<pt_mutex> first_guard;
        ft_unique_lock<pt_mutex> second_guard;
        int lock_error;
        bool separated;

        lock_error = aabb::lock_pair(first, second, first_guard, second_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            const_cast<aabb &>(first).set_error(lock_error);
            const_cast<aabb &>(second).set_error(lock_error);
            ft_errno = lock_error;
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
        const_cast<aabb &>(first).set_error(FT_ERR_SUCCESSS);
        const_cast<aabb &>(second).set_error(FT_ERR_SUCCESSS);
        result = true;
        if (separated)
            result = false;
    }
    ft_errno = FT_ERR_SUCCESSS;
    if (result)
        return (true);
    return (false);
}

bool    intersect_circle(const circle &first, const circle &second)
{
    bool result;

    result = false;
    {
        ft_unique_lock<pt_mutex> first_guard;
        ft_unique_lock<pt_mutex> second_guard;
        int lock_error;
        double  delta_x;
        double  delta_y;
        double  radius_sum;
        double  distance_squared;

        lock_error = circle::lock_pair(first, second, first_guard, second_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            const_cast<circle &>(first).set_error(lock_error);
            const_cast<circle &>(second).set_error(lock_error);
            ft_errno = lock_error;
            return (false);
        }
        delta_x = first._center_x - second._center_x;
        delta_y = first._center_y - second._center_y;
        radius_sum = first._radius + second._radius;
        distance_squared = delta_x * delta_x + delta_y * delta_y;
        const_cast<circle &>(first).set_error(FT_ERR_SUCCESSS);
        const_cast<circle &>(second).set_error(FT_ERR_SUCCESSS);
        result = true;
        if (distance_squared > radius_sum * radius_sum)
            result = false;
    }
    ft_errno = FT_ERR_SUCCESSS;
    if (result)
        return (true);
    return (false);
}

bool    intersect_sphere(const sphere &first, const sphere &second)
{
    bool result;

    result = false;
    {
        ft_unique_lock<pt_mutex> first_guard;
        ft_unique_lock<pt_mutex> second_guard;
        int lock_error;
        double  delta_x;
        double  delta_y;
        double  delta_z;
        double  radius_sum;
        double  distance_squared;

        lock_error = sphere::lock_pair(first, second, first_guard, second_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            const_cast<sphere &>(first).set_error(lock_error);
            const_cast<sphere &>(second).set_error(lock_error);
            ft_errno = lock_error;
            return (false);
        }
        delta_x = first._center_x - second._center_x;
        delta_y = first._center_y - second._center_y;
        delta_z = first._center_z - second._center_z;
        radius_sum = first._radius + second._radius;
        distance_squared = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;
        const_cast<sphere &>(first).set_error(FT_ERR_SUCCESSS);
        const_cast<sphere &>(second).set_error(FT_ERR_SUCCESSS);
        result = true;
        if (distance_squared > radius_sum * radius_sum)
            result = false;
    }
    ft_errno = FT_ERR_SUCCESSS;
    if (result)
        return (true);
    return (false);
}
