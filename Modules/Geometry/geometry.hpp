#ifndef GEOMETRY_HPP
# define GEOMETRY_HPP

#include "../Errno/errno.hpp"

class aabb;
class circle;
class sphere;
struct geometry_aabb3;
struct geometry_frustum;
struct geometry_plane;
struct geometry_ray;
struct geometry_raycast_hit;
struct geometry_sweep_hit;

ft_bool    intersect_aabb(const aabb &first, const aabb &second);
ft_bool    intersect_circle(const circle &first, const circle &second);
ft_bool    intersect_sphere(const sphere &first, const sphere &second);

#include "aabb.hpp"
#include "circle.hpp"
#include "sphere.hpp"
#include "geometry_3d.hpp"

#endif
