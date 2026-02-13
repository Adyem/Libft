#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_self_move_initialized_keeps_values,
    "vector2 self move on initialized object keeps values")
{
    vector2 vector_value(2.0, 5.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.move(vector_value));
    FT_ASSERT(math_fabs(vector_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y() - 5.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_self_move_initialized_keeps_values,
    "vector3 self move on initialized object keeps values")
{
    vector3 vector_value(2.0, 5.0, 7.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.move(vector_value));
    FT_ASSERT(math_fabs(vector_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_z() - 7.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_self_move_initialized_keeps_values,
    "vector4 self move on initialized object keeps values")
{
    vector4 vector_value(2.0, 5.0, 7.0, 11.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.move(vector_value));
    FT_ASSERT(math_fabs(vector_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_z() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_w() - 11.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_self_move_initialized_keeps_values,
    "matrix2 self move on initialized object keeps matrix values")
{
    matrix2 matrix_value(2.0, 0.0, 0.0, 3.0);
    vector2 vector_value(1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.move(matrix_value));
    vector2 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_self_move_initialized_keeps_values,
    "matrix3 self move on initialized object keeps matrix values")
{
    matrix3 matrix_value(2.0, 0.0, 0.0,
                         0.0, 3.0, 0.0,
                         0.0, 0.0, 4.0);
    vector3 vector_value(1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.move(matrix_value));
    vector3 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_self_move_initialized_keeps_values,
    "matrix4 self move on initialized object keeps matrix values")
{
    matrix4 matrix_value(2.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0, 4.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);
    vector4 vector_value(1.0, 1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.move(matrix_value));
    vector4 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_w() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_self_move_initialized_keeps_values,
    "quaternion self move on initialized object keeps values")
{
    quaternion quaternion_value(2.0, 3.0, 5.0, 7.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.move(quaternion_value));
    FT_ASSERT(math_fabs(quaternion_value.get_w() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_x() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_y() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_z() - 7.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}
