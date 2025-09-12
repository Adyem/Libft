#include "../Math/math.hpp"
#include "../System_utils/test_runner.hpp"

FT_TEST(test_vector3_dot, "vector3 dot product")
{
    vector3 a(1.0, 0.0, 0.0);
    vector3 b(0.0, 1.0, 0.0);

    FT_ASSERT(math_fabs(a.dot(b)) < 0.000001);
    return (1);
}

FT_TEST(test_vector3_cross, "vector3 cross product")
{
    vector3 a(1.0, 0.0, 0.0);
    vector3 b(0.0, 1.0, 0.0);
    vector3 c;

    c = a.cross(b);
    FT_ASSERT(math_fabs(c.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(c.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(c.get_z() - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_matrix4_identity_transform, "matrix4 identity transform")
{
    matrix4 identity;
    vector4 v(1.0, 2.0, 3.0, 1.0);
    vector4 r;

    r = identity.transform(v);
    FT_ASSERT(math_fabs(r.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(r.get_y() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(r.get_z() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(r.get_w() - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_quaternion_rotate_z, "quaternion rotate around z")
{
    double angle;
    double half;
    quaternion q;
    vector3 v(1.0, 0.0, 0.0);
    vector3 r;

    angle = math_deg2rad(90.0);
    half = angle / 2.0;
    q = quaternion(math_cos(half), 0.0, 0.0, ft_sin(half));
    r = q.transform(v);
    FT_ASSERT(math_fabs(r.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(r.get_y() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(r.get_z()) < 0.000001);
    return (1);
}
