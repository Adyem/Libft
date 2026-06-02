#include "geometry.hpp"

#include <cmath>
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static const double GEOMETRY_EPSILON = 0.0000001;

static double geometry_min_double(double first_value, double second_value)
{
    if (first_value < second_value)
    {
        return (first_value);
    }
    return (second_value);
}

static double geometry_max_double(double first_value, double second_value)
{
    if (first_value > second_value)
    {
        return (first_value);
    }
    return (second_value);
}

static double geometry_clamp_double(double value, double minimum_value,
    double maximum_value)
{
    if (value < minimum_value)
    {
        return (minimum_value);
    }
    if (value > maximum_value)
    {
        return (maximum_value);
    }
    return (value);
}

static int32_t geometry_initialize_vector(vector3 *target, double x_component,
    double y_component, double z_component)
{
    if (target == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    return (target->initialize(x_component, y_component, z_component));
}

static ft_bool geometry_ray_axis_interval(double origin_value,
    double direction_value, double minimum_value, double maximum_value,
    double *near_distance, double *far_distance)
{
    double inverse_direction;
    double first_distance;
    double second_distance;
    double swap_distance;

    if (std::fabs(direction_value) < GEOMETRY_EPSILON)
    {
        if (origin_value < minimum_value || origin_value > maximum_value)
        {
            return (FT_FALSE);
        }
        return (FT_TRUE);
    }
    inverse_direction = 1.0 / direction_value;
    first_distance = (minimum_value - origin_value) * inverse_direction;
    second_distance = (maximum_value - origin_value) * inverse_direction;
    if (first_distance > second_distance)
    {
        swap_distance = first_distance;
        first_distance = second_distance;
        second_distance = swap_distance;
    }
    *near_distance = geometry_max_double(*near_distance, first_distance);
    *far_distance = geometry_min_double(*far_distance, second_distance);
    if (*near_distance > *far_distance)
    {
        return (FT_FALSE);
    }
    return (FT_TRUE);
}

static int32_t geometry_set_hit_point(geometry_raycast_hit *hit,
    const geometry_ray &ray, double distance)
{
    double point_x;
    double point_y;
    double point_z;

    if (hit == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    point_x = ray.origin.get_x() + (ray.direction.get_x() * distance);
    point_y = ray.origin.get_y() + (ray.direction.get_y() * distance);
    point_z = ray.origin.get_z() + (ray.direction.get_z() * distance);
    hit->distance = distance;
    return (geometry_initialize_vector(&hit->point, point_x, point_y, point_z));
}

static int32_t geometry_set_aabb_hit_normal(geometry_raycast_hit *hit,
    const geometry_ray &ray, const geometry_aabb3 &box)
{
    double point_x;
    double point_y;
    double point_z;
    double normal_x;
    double normal_y;
    double normal_z;

    if (hit == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    point_x = hit->point.get_x();
    point_y = hit->point.get_y();
    point_z = hit->point.get_z();
    normal_x = 0.0;
    normal_y = 0.0;
    normal_z = 0.0;
    if (std::fabs(point_x - box.minimum.get_x()) < GEOMETRY_EPSILON)
        normal_x = -1.0;
    else if (std::fabs(point_x - box.maximum.get_x()) < GEOMETRY_EPSILON)
        normal_x = 1.0;
    else if (std::fabs(point_y - box.minimum.get_y()) < GEOMETRY_EPSILON)
        normal_y = -1.0;
    else if (std::fabs(point_y - box.maximum.get_y()) < GEOMETRY_EPSILON)
        normal_y = 1.0;
    else if (std::fabs(point_z - box.minimum.get_z()) < GEOMETRY_EPSILON)
        normal_z = -1.0;
    else if (std::fabs(point_z - box.maximum.get_z()) < GEOMETRY_EPSILON)
        normal_z = 1.0;
    else if (std::fabs(ray.direction.get_x()) >= GEOMETRY_EPSILON)
        normal_x = -ray.direction.get_x() / std::fabs(ray.direction.get_x());
    return (geometry_initialize_vector(&hit->normal, normal_x, normal_y,
        normal_z));
}

ft_bool geometry_ray_intersect_plane(const geometry_ray &ray,
    const geometry_plane &plane, double *distance)
{
    double denominator;
    double numerator;
    double result_distance;

    denominator = ray.direction.dot(plane.normal);
    if (std::fabs(denominator) < GEOMETRY_EPSILON)
    {
        return (FT_FALSE);
    }
    numerator = -(ray.origin.dot(plane.normal) + plane.distance);
    result_distance = numerator / denominator;
    if (result_distance < 0.0)
    {
        return (FT_FALSE);
    }
    if (distance != ft_nullptr)
    {
        *distance = result_distance;
    }
    return (FT_TRUE);
}

ft_bool geometry_ray_intersect_triangle(const geometry_ray &ray,
    const vector3 &vertex_a, const vector3 &vertex_b, const vector3 &vertex_c,
    geometry_raycast_hit *hit)
{
    vector3 edge_ab(vertex_b.subtract(vertex_a));
    vector3 edge_ac(vertex_c.subtract(vertex_a));
    vector3 p_vector(ray.direction.cross(edge_ac));
    vector3 t_vector(ray.origin.subtract(vertex_a));
    vector3 q_vector;
    double determinant;
    double inverse_determinant;
    double barycentric_u;
    double barycentric_v;
    double distance;
    vector3 normal;

    determinant = edge_ab.dot(p_vector);
    if (std::fabs(determinant) < GEOMETRY_EPSILON)
    {
        return (FT_FALSE);
    }
    inverse_determinant = 1.0 / determinant;
    barycentric_u = t_vector.dot(p_vector) * inverse_determinant;
    if (barycentric_u < 0.0 || barycentric_u > 1.0)
    {
        return (FT_FALSE);
    }
    if (q_vector.initialize(t_vector.cross(edge_ab)) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    barycentric_v = ray.direction.dot(q_vector) * inverse_determinant;
    if (barycentric_v < 0.0 || barycentric_u + barycentric_v > 1.0)
    {
        return (FT_FALSE);
    }
    distance = edge_ac.dot(q_vector) * inverse_determinant;
    if (distance < 0.0)
    {
        return (FT_FALSE);
    }
    if (geometry_set_hit_point(hit, ray, distance) != FT_ERR_SUCCESS)
    {
        return (FT_FALSE);
    }
    if (hit != ft_nullptr)
    {
        if (normal.initialize(edge_ab.cross(edge_ac).normalize()) != FT_ERR_SUCCESS)
            return (FT_FALSE);
        if (hit->normal.initialize(normal) != FT_ERR_SUCCESS)
        {
            return (FT_FALSE);
        }
    }
    return (FT_TRUE);
}

ft_bool geometry_ray_intersect_aabb3(const geometry_ray &ray,
    const geometry_aabb3 &box, geometry_raycast_hit *hit)
{
    double near_distance;
    double far_distance;

    near_distance = 0.0;
    far_distance = 1.0e30;
    if (geometry_ray_axis_interval(ray.origin.get_x(), ray.direction.get_x(),
            box.minimum.get_x(), box.maximum.get_x(), &near_distance,
            &far_distance) == FT_FALSE)
        return (FT_FALSE);
    if (geometry_ray_axis_interval(ray.origin.get_y(), ray.direction.get_y(),
            box.minimum.get_y(), box.maximum.get_y(), &near_distance,
            &far_distance) == FT_FALSE)
        return (FT_FALSE);
    if (geometry_ray_axis_interval(ray.origin.get_z(), ray.direction.get_z(),
            box.minimum.get_z(), box.maximum.get_z(), &near_distance,
            &far_distance) == FT_FALSE)
        return (FT_FALSE);
    if (geometry_set_hit_point(hit, ray, near_distance) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (geometry_set_aabb_hit_normal(hit, ray, box) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    return (FT_TRUE);
}

ft_bool geometry_sphere_intersect_aabb3(const sphere &shape,
    const geometry_aabb3 &box)
{
    double closest_x;
    double closest_y;
    double closest_z;
    double delta_x;
    double delta_y;
    double delta_z;
    double radius;

    closest_x = geometry_clamp_double(shape.get_center_x(), box.minimum.get_x(),
        box.maximum.get_x());
    closest_y = geometry_clamp_double(shape.get_center_y(), box.minimum.get_y(),
        box.maximum.get_y());
    closest_z = geometry_clamp_double(shape.get_center_z(), box.minimum.get_z(),
        box.maximum.get_z());
    delta_x = shape.get_center_x() - closest_x;
    delta_y = shape.get_center_y() - closest_y;
    delta_z = shape.get_center_z() - closest_z;
    radius = shape.get_radius();
    if ((delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z)
        <= radius * radius)
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

ft_bool geometry_swept_sphere_intersect_aabb3(const sphere &shape,
    const vector3 &velocity, const geometry_aabb3 &box, geometry_sweep_hit *hit)
{
    geometry_ray         ray;
    geometry_aabb3       expanded_box;
    geometry_raycast_hit ray_hit;
    double               radius;

    radius = shape.get_radius();
    if (geometry_sphere_intersect_aabb3(shape, box) == FT_TRUE)
    {
        if (hit != ft_nullptr)
        {
            hit->time = 0.0;
            if (hit->point.initialize(shape.get_center_x(), shape.get_center_y(),
                    shape.get_center_z()) != FT_ERR_SUCCESS)
                return (FT_FALSE);
            if (hit->normal.initialize(0.0, 0.0, 0.0) != FT_ERR_SUCCESS)
                return (FT_FALSE);
        }
        return (FT_TRUE);
    }
    if (ray.origin.initialize(shape.get_center_x(), shape.get_center_y(),
            shape.get_center_z()) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (ray.direction.initialize(velocity) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (expanded_box.minimum.initialize(box.minimum.get_x() - radius,
            box.minimum.get_y() - radius, box.minimum.get_z() - radius)
        != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (expanded_box.maximum.initialize(box.maximum.get_x() + radius,
            box.maximum.get_y() + radius, box.maximum.get_z() + radius)
        != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (geometry_ray_intersect_aabb3(ray, expanded_box, &ray_hit) == FT_FALSE)
        return (FT_FALSE);
    if (ray_hit.distance < 0.0 || ray_hit.distance > 1.0)
        return (FT_FALSE);
    if (hit != ft_nullptr)
    {
        hit->time = ray_hit.distance;
        if (hit->point.initialize(ray_hit.point) != FT_ERR_SUCCESS)
            return (FT_FALSE);
        if (hit->normal.initialize(ray_hit.normal) != FT_ERR_SUCCESS)
            return (FT_FALSE);
    }
    return (FT_TRUE);
}

ft_bool geometry_frustum_contains_point(const geometry_frustum &frustum,
    const vector3 &point)
{
    int32_t plane_index;
    double  signed_distance;

    plane_index = 0;
    while (plane_index < 6)
    {
        signed_distance = point.dot(frustum.planes[plane_index].normal)
            + frustum.planes[plane_index].distance;
        if (signed_distance < 0.0)
        {
            return (FT_FALSE);
        }
        plane_index = plane_index + 1;
    }
    return (FT_TRUE);
}

ft_bool geometry_frustum_intersect_aabb3(const geometry_frustum &frustum,
    const geometry_aabb3 &box)
{
    int32_t plane_index;
    double normal_x;
    double normal_y;
    double normal_z;
    double vertex_x;
    double vertex_y;
    double vertex_z;
    double signed_distance;

    plane_index = 0;
    while (plane_index < 6)
    {
        normal_x = frustum.planes[plane_index].normal.get_x();
        normal_y = frustum.planes[plane_index].normal.get_y();
        normal_z = frustum.planes[plane_index].normal.get_z();
        vertex_x = box.minimum.get_x();
        vertex_y = box.minimum.get_y();
        vertex_z = box.minimum.get_z();
        if (normal_x >= 0.0)
            vertex_x = box.maximum.get_x();
        if (normal_y >= 0.0)
            vertex_y = box.maximum.get_y();
        if (normal_z >= 0.0)
            vertex_z = box.maximum.get_z();
        signed_distance = (vertex_x * normal_x) + (vertex_y * normal_y)
            + (vertex_z * normal_z)
            + frustum.planes[plane_index].distance;
        if (signed_distance < 0.0)
        {
            return (FT_FALSE);
        }
        plane_index = plane_index + 1;
    }
    return (FT_TRUE);
}
