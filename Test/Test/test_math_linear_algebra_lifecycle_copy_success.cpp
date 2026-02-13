#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector3_initialize_copy_success,
    "vector3 initialize(copy) succeeds and copies values")
{
    vector3 source_value(2.0, 4.0, 6.0);
    vector3 destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 6.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector4_initialize_copy_success,
    "vector4 initialize(copy) succeeds and copies values")
{
    vector4 source_value(2.0, 4.0, 6.0, 8.0);
    vector4 destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_w() - 8.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_initialize_copy_success,
    "matrix3 initialize(copy) succeeds and copies values")
{
    matrix3 source_value(2.0, 0.0, 0.0,
                         0.0, 3.0, 0.0,
                         0.0, 0.0, 4.0);
    matrix3 destination_value;
    vector3 vector_value(1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    vector3 transformed_value = destination_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_initialize_copy_success,
    "matrix4 initialize(copy) succeeds and copies values")
{
    matrix4 source_value(2.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0, 4.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);
    matrix4 destination_value;
    vector4 vector_value(1.0, 1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    vector4 transformed_value = destination_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_w() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_initialize_copy_success,
    "quaternion initialize(copy) succeeds and copies values")
{
    quaternion source_value(2.0, 3.0, 4.0, 5.0);
    quaternion destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(destination_value.get_w() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_x() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 5.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}
