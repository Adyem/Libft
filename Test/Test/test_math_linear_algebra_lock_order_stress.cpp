#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_lock_order_stress,
    "vector2 repeated cross-object operations remain stable with thread safety")
{
    vector2 left_value(1.0, 2.0);
    vector2 right_value(3.0, 4.0);
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    index = 0;
    while (index < 512)
    {
        if ((index % 2) == 0)
        {
            vector2 result_value = left_value.add(right_value);
            FT_ASSERT(math_fabs(result_value.get_x() - 4.0) < 0.000001);
            FT_ASSERT(math_fabs(result_value.get_y() - 6.0) < 0.000001);
        }
        else
        {
            vector2 result_value = right_value.add(left_value);
            FT_ASSERT(math_fabs(result_value.get_x() - 4.0) < 0.000001);
            FT_ASSERT(math_fabs(result_value.get_y() - 6.0) < 0.000001);
        }
        index++;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_vector3_lock_order_stress,
    "vector3 repeated cross-object operations remain stable with thread safety")
{
    vector3 left_value(1.0, 0.0, 0.0);
    vector3 right_value(0.0, 1.0, 0.0);
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    index = 0;
    while (index < 512)
    {
        if ((index % 2) == 0)
        {
            vector3 result_value = left_value.cross(right_value);
            FT_ASSERT(math_fabs(result_value.get_z() - 1.0) < 0.000001);
        }
        else
        {
            vector3 result_value = right_value.cross(left_value);
            FT_ASSERT(math_fabs(result_value.get_z() + 1.0) < 0.000001);
        }
        index++;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_vector4_lock_order_stress,
    "vector4 repeated cross-object operations remain stable with thread safety")
{
    vector4 left_value(1.0, 2.0, 3.0, 4.0);
    vector4 right_value(5.0, 6.0, 7.0, 8.0);
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    index = 0;
    while (index < 512)
    {
        double dot_value;

        if ((index % 2) == 0)
            dot_value = left_value.dot(right_value);
        else
            dot_value = right_value.dot(left_value);
        FT_ASSERT(math_fabs(dot_value - 70.0) < 0.000001);
        index++;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_lock_order_stress,
    "matrix2 repeated cross-object operations remain stable with thread safety")
{
    matrix2 left_value(1.0, 2.0, 3.0, 4.0);
    matrix2 right_value(2.0, 0.0, 0.0, 2.0);
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    index = 0;
    while (index < 512)
    {
        if ((index % 2) == 0)
        {
            matrix2 result_value = left_value.multiply(right_value);
            vector2 probe(1.0, 1.0);
            vector2 transformed_value = result_value.transform(probe);
            FT_ASSERT(math_fabs(transformed_value.get_x() - 6.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_y() - 14.0) < 0.000001);
            FT_ASSERT_EQ(FT_ERR_SUCCESS, probe.destroy());
        }
        else
        {
            matrix2 result_value = right_value.multiply(left_value);
            vector2 probe(1.0, 1.0);
            vector2 transformed_value = result_value.transform(probe);
            FT_ASSERT(math_fabs(transformed_value.get_x() - 6.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_y() - 14.0) < 0.000001);
            FT_ASSERT_EQ(FT_ERR_SUCCESS, probe.destroy());
        }
        index++;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_lock_order_stress,
    "matrix3 repeated cross-object operations remain stable with thread safety")
{
    matrix3 left_value(1.0, 2.0, 3.0,
                       4.0, 5.0, 6.0,
                       7.0, 8.0, 9.0);
    matrix3 right_value(1.0, 0.0, 0.0,
                        0.0, 1.0, 0.0,
                        0.0, 0.0, 1.0);
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    index = 0;
    while (index < 512)
    {
        if ((index % 2) == 0)
        {
            matrix3 result_value = left_value.multiply(right_value);
            vector3 probe(1.0, 1.0, 1.0);
            vector3 transformed_value = result_value.transform(probe);
            FT_ASSERT(math_fabs(transformed_value.get_x() - 6.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_y() - 15.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_z() - 24.0) < 0.000001);
            FT_ASSERT_EQ(FT_ERR_SUCCESS, probe.destroy());
        }
        else
        {
            matrix3 result_value = right_value.multiply(left_value);
            vector3 probe(1.0, 1.0, 1.0);
            vector3 transformed_value = result_value.transform(probe);
            FT_ASSERT(math_fabs(transformed_value.get_x() - 6.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_y() - 15.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_z() - 24.0) < 0.000001);
            FT_ASSERT_EQ(FT_ERR_SUCCESS, probe.destroy());
        }
        index++;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_lock_order_stress,
    "matrix4 repeated cross-object operations remain stable with thread safety")
{
    matrix4 left_value(1.0, 0.0, 0.0, 2.0,
                       0.0, 1.0, 0.0, 3.0,
                       0.0, 0.0, 1.0, 4.0,
                       0.0, 0.0, 0.0, 1.0);
    matrix4 right_value(1.0, 0.0, 0.0, 0.0,
                        0.0, 1.0, 0.0, 0.0,
                        0.0, 0.0, 1.0, 0.0,
                        0.0, 0.0, 0.0, 1.0);
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    index = 0;
    while (index < 512)
    {
        if ((index % 2) == 0)
        {
            matrix4 result_value = left_value.multiply(right_value);
            vector4 probe(1.0, 1.0, 1.0, 1.0);
            vector4 transformed_value = result_value.transform(probe);
            FT_ASSERT(math_fabs(transformed_value.get_x() - 3.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_y() - 4.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_z() - 5.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_w() - 1.0) < 0.000001);
            FT_ASSERT_EQ(FT_ERR_SUCCESS, probe.destroy());
        }
        else
        {
            matrix4 result_value = right_value.multiply(left_value);
            vector4 probe(1.0, 1.0, 1.0, 1.0);
            vector4 transformed_value = result_value.transform(probe);
            FT_ASSERT(math_fabs(transformed_value.get_x() - 3.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_y() - 4.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_z() - 5.0) < 0.000001);
            FT_ASSERT(math_fabs(transformed_value.get_w() - 1.0) < 0.000001);
            FT_ASSERT_EQ(FT_ERR_SUCCESS, probe.destroy());
        }
        index++;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_lock_order_stress,
    "quaternion repeated cross-object operations remain stable with thread safety")
{
    quaternion left_value(1.0, 2.0, 3.0, 4.0);
    quaternion right_value(5.0, 6.0, 7.0, 8.0);
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    index = 0;
    while (index < 512)
    {
        if ((index % 2) == 0)
        {
            quaternion result_value = left_value.multiply(right_value);
            double length_value = result_value.length();

            FT_ASSERT(length_value > 0.0);
        }
        else
        {
            quaternion result_value = right_value.multiply(left_value);
            double length_value = result_value.length();

            FT_ASSERT(length_value > 0.0);
        }
        index++;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}
