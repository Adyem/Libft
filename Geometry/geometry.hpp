#ifndef GEOMETRY_HPP
# define GEOMETRY_HPP

class aabb;
class circle;
class sphere;

bool    intersect_aabb(const aabb &first, const aabb &second);
bool    intersect_circle(const circle &first, const circle &second);
bool    intersect_sphere(const sphere &first, const sphere &second);

# include "geometry_aabb.hpp"
# include "geometry_circle.hpp"
# include "geometry_sphere.hpp"

#endif
