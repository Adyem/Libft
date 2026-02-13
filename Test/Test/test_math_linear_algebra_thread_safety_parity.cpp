#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_operation_parity_thread_safety_on_off,
    "vector2 add returns same result with thread safety on and off")
{
    vector2 left_value(1.5, -2.0);
    vector2 right_value(3.0, 4.5);
    vector2 result_without_thread_safety = left_value.add(right_value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    vector2 result_with_thread_safety = left_value.add(right_value);

    FT_ASSERT(math_fabs(result_without_thread_safety.get_x()
            - result_with_thread_safety.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_y()
            - result_with_thread_safety.get_y()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_vector3_operation_parity_thread_safety_on_off,
    "vector3 cross returns same result with thread safety on and off")
{
    vector3 left_value(1.0, 2.0, 3.0);
    vector3 right_value(4.0, 5.0, 6.0);
    vector3 result_without_thread_safety = left_value.cross(right_value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    vector3 result_with_thread_safety = left_value.cross(right_value);

    FT_ASSERT(math_fabs(result_without_thread_safety.get_x()
            - result_with_thread_safety.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_y()
            - result_with_thread_safety.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_z()
            - result_with_thread_safety.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_vector4_operation_parity_thread_safety_on_off,
    "vector4 dot returns same result with thread safety on and off")
{
    vector4 left_value(1.0, 2.0, 3.0, 4.0);
    vector4 right_value(5.0, 6.0, 7.0, 8.0);
    double result_without_thread_safety;
    double result_with_thread_safety;

    result_without_thread_safety = left_value.dot(right_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    result_with_thread_safety = left_value.dot(right_value);
    FT_ASSERT(math_fabs(result_without_thread_safety - result_with_thread_safety) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_operation_parity_thread_safety_on_off,
    "matrix2 transform returns same result with thread safety on and off")
{
    matrix2 matrix_value(2.0, 1.0, 0.0, 3.0);
    vector2 vector_value(1.0, 2.0);
    vector2 result_without_thread_safety = matrix_value.transform(vector_value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    vector2 result_with_thread_safety = matrix_value.transform(vector_value);

    FT_ASSERT(math_fabs(result_without_thread_safety.get_x()
            - result_with_thread_safety.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_y()
            - result_with_thread_safety.get_y()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_operation_parity_thread_safety_on_off,
    "matrix3 transform returns same result with thread safety on and off")
{
    matrix3 matrix_value(1.0, 2.0, 3.0,
                         0.0, 1.0, 4.0,
                         5.0, 6.0, 0.0);
    vector3 vector_value(1.0, 2.0, 3.0);
    vector3 result_without_thread_safety = matrix_value.transform(vector_value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    vector3 result_with_thread_safety = matrix_value.transform(vector_value);

    FT_ASSERT(math_fabs(result_without_thread_safety.get_x()
            - result_with_thread_safety.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_y()
            - result_with_thread_safety.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_z()
            - result_with_thread_safety.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_operation_parity_thread_safety_on_off,
    "matrix4 transform returns same result with thread safety on and off")
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 2.0,
                         0.0, 1.0, 0.0, 3.0,
                         0.0, 0.0, 1.0, 4.0,
                         0.0, 0.0, 0.0, 1.0);
    vector4 vector_value(1.0, 1.0, 1.0, 1.0);
    vector4 result_without_thread_safety = matrix_value.transform(vector_value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    vector4 result_with_thread_safety = matrix_value.transform(vector_value);

    FT_ASSERT(math_fabs(result_without_thread_safety.get_x()
            - result_with_thread_safety.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_y()
            - result_with_thread_safety.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_z()
            - result_with_thread_safety.get_z()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_w()
            - result_with_thread_safety.get_w()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_operation_parity_thread_safety_on_off,
    "quaternion normalize returns same result with thread safety on and off")
{
    quaternion quaternion_value(1.0, 2.0, 3.0, 4.0);
    quaternion result_without_thread_safety = quaternion_value.normalize();

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.enable_thread_safety());
    quaternion result_with_thread_safety = quaternion_value.normalize();

    FT_ASSERT(math_fabs(result_without_thread_safety.get_w()
            - result_with_thread_safety.get_w()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_x()
            - result_with_thread_safety.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_y()
            - result_with_thread_safety.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(result_without_thread_safety.get_z()
            - result_with_thread_safety.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}
