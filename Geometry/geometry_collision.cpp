#include "geometry.hpp"

bool    intersect_aabb(const aabb &first, const aabb &second)
{
    if (first.maximum_x < second.minimum_x)
        return (false);
    if (first.minimum_x > second.maximum_x)
        return (false);
    if (first.maximum_y < second.minimum_y)
        return (false);
    if (first.minimum_y > second.maximum_y)
        return (false);
    return (true);
}

bool    intersect_circle(const circle &first, const circle &second)
{
    double  delta_x;
    double  delta_y;
    double  radius_sum;
    double  distance_squared;

    delta_x = first.center_x - second.center_x;
    delta_y = first.center_y - second.center_y;
    radius_sum = first.radius + second.radius;
    distance_squared = delta_x * delta_x + delta_y * delta_y;
    if (distance_squared > radius_sum * radius_sum)
        return (false);
    return (true);
}

bool    intersect_sphere(const sphere &first, const sphere &second)
{
    double  delta_x;
    double  delta_y;
    double  delta_z;
    double  radius_sum;
    double  distance_squared;

    delta_x = first.center_x - second.center_x;
    delta_y = first.center_y - second.center_y;
    delta_z = first.center_z - second.center_z;
    radius_sum = first.radius + second.radius;
    distance_squared = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;
    if (distance_squared > radius_sum * radius_sum)
        return (false);
    return (true);
}
