#include "../../Geometry/geometry.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_intersect_aabb_overlap, "intersect_aabb detects overlapping boxes")
{
    aabb first;
    aabb second;

    first.minimum_x = 0.0;
    first.minimum_y = 0.0;
    first.maximum_x = 5.0;
    first.maximum_y = 5.0;
    second.minimum_x = 3.0;
    second.minimum_y = 3.0;
    second.maximum_x = 8.0;
    second.maximum_y = 8.0;
    FT_ASSERT(intersect_aabb(first, second));
    FT_ASSERT(intersect_aabb(second, first));
    return (1);
}

FT_TEST(test_intersect_aabb_separated, "intersect_aabb returns false when separated on axis")
{
    aabb first;
    aabb second;

    first.minimum_x = -2.0;
    first.minimum_y = -2.0;
    first.maximum_x = 2.0;
    first.maximum_y = 2.0;
    second.minimum_x = 3.0;
    second.minimum_y = -1.0;
    second.maximum_x = 6.0;
    second.maximum_y = 1.0;
    FT_ASSERT_EQ(false, intersect_aabb(first, second));
    FT_ASSERT_EQ(false, intersect_aabb(second, first));
    return (1);
}

FT_TEST(test_intersect_aabb_touching_edge, "intersect_aabb treats shared boundary as collision")
{
    aabb first;
    aabb second;

    first.minimum_x = 0.0;
    first.minimum_y = 0.0;
    first.maximum_x = 4.0;
    first.maximum_y = 4.0;
    second.minimum_x = 4.0;
    second.minimum_y = 1.0;
    second.maximum_x = 7.0;
    second.maximum_y = 3.0;
    FT_ASSERT(intersect_aabb(first, second));
    FT_ASSERT(intersect_aabb(second, first));
    return (1);
}

FT_TEST(test_intersect_circle_overlap, "intersect_circle detects overlapping circles")
{
    circle first;
    circle second;

    first.center_x = 0.0;
    first.center_y = 0.0;
    first.radius = 4.0;
    second.center_x = 3.0;
    second.center_y = 0.0;
    second.radius = 3.0;
    FT_ASSERT(intersect_circle(first, second));
    FT_ASSERT(intersect_circle(second, first));
    return (1);
}

FT_TEST(test_intersect_circle_separated, "intersect_circle returns false when distance exceeds radii")
{
    circle first;
    circle second;

    first.center_x = -5.0;
    first.center_y = -5.0;
    first.radius = 2.0;
    second.center_x = 2.0;
    second.center_y = 2.0;
    second.radius = 1.5;
    FT_ASSERT_EQ(false, intersect_circle(first, second));
    FT_ASSERT_EQ(false, intersect_circle(second, first));
    return (1);
}

FT_TEST(test_intersect_circle_touching, "intersect_circle treats tangential contact as intersection")
{
    circle first;
    circle second;

    first.center_x = 0.0;
    first.center_y = 0.0;
    first.radius = 3.0;
    second.center_x = 4.0;
    second.center_y = 0.0;
    second.radius = 1.0;
    FT_ASSERT(intersect_circle(first, second));
    FT_ASSERT(intersect_circle(second, first));
    return (1);
}

FT_TEST(test_intersect_sphere_overlap, "intersect_sphere detects overlapping spheres")
{
    sphere first;
    sphere second;

    first.center_x = 1.0;
    first.center_y = 2.0;
    first.center_z = 3.0;
    first.radius = 5.0;
    second.center_x = 4.0;
    second.center_y = 3.0;
    second.center_z = 5.0;
    second.radius = 4.0;
    FT_ASSERT(intersect_sphere(first, second));
    FT_ASSERT(intersect_sphere(second, first));
    return (1);
}

FT_TEST(test_intersect_sphere_separated, "intersect_sphere returns false when centers far apart")
{
    sphere first;
    sphere second;

    first.center_x = -4.0;
    first.center_y = -4.0;
    first.center_z = -4.0;
    first.radius = 1.0;
    second.center_x = 4.0;
    second.center_y = 4.0;
    second.center_z = 4.0;
    second.radius = 1.0;
    FT_ASSERT_EQ(false, intersect_sphere(first, second));
    FT_ASSERT_EQ(false, intersect_sphere(second, first));
    return (1);
}

FT_TEST(test_intersect_sphere_touching, "intersect_sphere treats tangential contact as collision")
{
    sphere first;
    sphere second;

    first.center_x = 0.0;
    first.center_y = 0.0;
    first.center_z = 0.0;
    first.radius = 2.5;
    second.center_x = 5.0;
    second.center_y = 0.0;
    second.center_z = 0.0;
    second.radius = 2.5;
    FT_ASSERT(intersect_sphere(first, second));
    FT_ASSERT(intersect_sphere(second, first));
    return (1);
}
