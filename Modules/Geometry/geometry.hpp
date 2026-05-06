#ifndef GEOMETRY_HPP
# define GEOMETRY_HPP

#include "../Errno/errno.hpp"

class aabb;
class circle;
class sphere;

ft_bool    intersect_aabb(const aabb &first, const aabb &second);
ft_bool    intersect_circle(const circle &first, const circle &second);
ft_bool    intersect_sphere(const sphere &first, const sphere &second);

#include "aabb.hpp"
#include "circle.hpp"
#include "sphere.hpp"

#endif
