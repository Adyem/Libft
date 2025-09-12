#include "../Math/math.hpp"
#include "../System_utils/test_runner.hpp"

FT_TEST(test_quaternion_add, "quaternion add")
{
    quaternion first_quaternion(1.0, 2.0, 3.0, 4.0);
    quaternion second_quaternion(0.5, -1.0, 2.0, -0.5);
    quaternion result;

    result = first_quaternion.add(second_quaternion);
    FT_ASSERT(math_fabs(result.get_w() - 1.5) < 0.000001);
    FT_ASSERT(math_fabs(result.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(result.get_y() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(result.get_z() - 3.5) < 0.000001);
    return (1);
}

FT_TEST(test_quaternion_multiply, "quaternion multiply")
{
    quaternion first_quaternion(1.0, 2.0, 3.0, 4.0);
    quaternion second_quaternion(5.0, 6.0, 7.0, 8.0);
    quaternion result;

    result = first_quaternion.multiply(second_quaternion);
    FT_ASSERT(math_fabs(result.get_w() + 60.0) < 0.000001);
    FT_ASSERT(math_fabs(result.get_x() - 12.0) < 0.000001);
    FT_ASSERT(math_fabs(result.get_y() - 30.0) < 0.000001);
    FT_ASSERT(math_fabs(result.get_z() - 24.0) < 0.000001);
    return (1);
}

FT_TEST(test_quaternion_conjugate, "quaternion conjugate")
{
    quaternion value(1.0, -2.0, 3.0, -4.0);
    quaternion result;

    result = value.conjugate();
    FT_ASSERT(math_fabs(result.get_w() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(result.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(result.get_y() + 3.0) < 0.000001);
    FT_ASSERT(math_fabs(result.get_z() - 4.0) < 0.000001);
    return (1);
}

FT_TEST(test_quaternion_normalize, "quaternion normalize")
{
    quaternion value(1.0, 2.0, 3.0, 4.0);
    quaternion normalized;
    double length_value;

    normalized = value.normalize();
    length_value = math_sqrt(normalized.get_w() * normalized.get_w()
        + normalized.get_x() * normalized.get_x()
        + normalized.get_y() * normalized.get_y()
        + normalized.get_z() * normalized.get_z());
    FT_ASSERT(math_fabs(length_value - 1.0) < 0.000001);
    return (1);
}
