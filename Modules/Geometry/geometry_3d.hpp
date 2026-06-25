#ifndef GEOMETRY_3D_HPP
# define GEOMETRY_3D_HPP

#include "../Errno/errno.hpp"
#include "../Math/vector3.hpp"
#include "sphere.hpp"

struct geometry_ray
{
    vector3 origin;
    vector3 direction;
};

struct geometry_plane
{
    vector3 normal;
    double  distance;
};

struct geometry_aabb3
{
    vector3 minimum;
    vector3 maximum;
};

struct geometry_frustum
{
    geometry_plane planes[6];
};

struct geometry_raycast_hit
{
    double  distance;
    vector3 point;
    vector3 normal;
};

struct geometry_sweep_hit
{
    double  time;
    vector3 point;
    vector3 normal;
};

ft_bool geometry_ray_intersect_plane(const geometry_ray &ray,
            const geometry_plane &plane, double *distance);
ft_bool geometry_ray_intersect_triangle(const geometry_ray &ray,
            const vector3 &vertex_a, const vector3 &vertex_b,
            const vector3 &vertex_c, geometry_raycast_hit *hit);
ft_bool geometry_ray_intersect_aabb3(const geometry_ray &ray,
            const geometry_aabb3 &box, geometry_raycast_hit *hit);
ft_bool geometry_sphere_intersect_aabb3(const sphere &shape,
            const geometry_aabb3 &box);
ft_bool geometry_swept_sphere_intersect_aabb3(const sphere &shape,
            const vector3 &velocity, const geometry_aabb3 &box,
            geometry_sweep_hit *hit);
ft_bool geometry_frustum_contains_point(const geometry_frustum &frustum,
            const vector3 &point);
ft_bool geometry_frustum_intersect_aabb3(const geometry_frustum &frustum,
            const geometry_aabb3 &box);

#endif
