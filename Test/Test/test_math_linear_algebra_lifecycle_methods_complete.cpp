#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_initialize_default_on_uninitialized,
    "vector2 initialize() sets default zero state")
{
    vector2 vector_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.initialize());
    FT_ASSERT(math_fabs(vector_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_initialize_default_on_uninitialized,
    "vector3 initialize() sets default zero state")
{
    vector3 vector_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.initialize());
    FT_ASSERT(math_fabs(vector_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_initialize_default_on_uninitialized,
    "vector4 initialize() sets default zero state")
{
    vector4 vector_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.initialize());
    FT_ASSERT(math_fabs(vector_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_z()) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_w()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_initialize_default_on_uninitialized,
    "matrix2 initialize() sets identity state")
{
    matrix2 matrix_value;
    vector2 vector_value(2.0, 3.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.initialize());
    vector2 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_initialize_default_on_uninitialized,
    "matrix3 initialize() sets identity state")
{
    matrix3 matrix_value;
    vector3 vector_value(2.0, 3.0, 4.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.initialize());
    vector3 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_initialize_default_on_uninitialized,
    "matrix4 initialize() sets identity state")
{
    matrix4 matrix_value;
    vector4 vector_value(2.0, 3.0, 4.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.initialize());
    vector4 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_w() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_initialize_default_on_uninitialized,
    "quaternion initialize() sets identity state")
{
    quaternion quaternion_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.initialize());
    FT_ASSERT(math_fabs(quaternion_value.get_w() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}

FT_TEST(test_vector2_initialize_move_success,
    "vector2 initialize(move) transfers values")
{
    vector2 source_value(4.0, 5.0);
    vector2 destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<vector2 &&>(source_value)));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector4_initialize_move_success,
    "vector4 initialize(move) transfers values")
{
    vector4 source_value(4.0, 5.0, 6.0, 7.0);
    vector4 destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<vector4 &&>(source_value)));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_w() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_z()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_w()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_initialize_move_success,
    "matrix2 initialize(move) transfers values")
{
    matrix2 source_value(2.0, 0.0, 0.0, 3.0);
    matrix2 destination_value;
    vector2 vector_value(1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<matrix2 &&>(source_value)));
    vector2 destination_transform = destination_value.transform(vector_value);
    vector2 source_transform = source_value.transform(vector_value);
    FT_ASSERT(math_fabs(destination_transform.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_transform.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_y() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_initialize_move_success,
    "matrix3 initialize(move) transfers values")
{
    matrix3 source_value(2.0, 0.0, 0.0,
                         0.0, 3.0, 0.0,
                         0.0, 0.0, 4.0);
    matrix3 destination_value;
    vector3 vector_value(1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<matrix3 &&>(source_value)));
    vector3 destination_transform = destination_value.transform(vector_value);
    vector3 source_transform = source_value.transform(vector_value);
    FT_ASSERT(math_fabs(destination_transform.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_transform.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_transform.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_y() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_z() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_initialize_move_success,
    "quaternion initialize(move) transfers values")
{
    quaternion source_value(2.0, 3.0, 4.0, 5.0);
    quaternion destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<quaternion &&>(source_value)));
    FT_ASSERT(math_fabs(destination_value.get_w() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_x() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_w() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector2_move_method_success,
    "vector2 move method transfers values")
{
    vector2 source_value(3.0, 7.0);
    vector2 destination_value(0.0, 0.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector3_move_method_success,
    "vector3 move method transfers values")
{
    vector3 source_value(3.0, 7.0, 9.0);
    vector3 destination_value(0.0, 0.0, 0.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 9.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_move_method_success,
    "matrix2 move method transfers values")
{
    matrix2 source_value(2.0, 0.0, 0.0, 3.0);
    matrix2 destination_value(1.0, 0.0, 0.0, 1.0);
    vector2 vector_value(1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    vector2 destination_transform = destination_value.transform(vector_value);
    vector2 source_transform = source_value.transform(vector_value);
    FT_ASSERT(math_fabs(destination_transform.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_transform.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_y() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_move_method_success,
    "matrix4 move method transfers values")
{
    matrix4 source_value(2.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0, 4.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);
    matrix4 destination_value(1.0, 0.0, 0.0, 0.0,
                              0.0, 1.0, 0.0, 0.0,
                              0.0, 0.0, 1.0, 0.0,
                              0.0, 0.0, 0.0, 1.0);
    vector4 vector_value(1.0, 1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    vector4 destination_transform = destination_value.transform(vector_value);
    vector4 source_transform = source_value.transform(vector_value);
    FT_ASSERT(math_fabs(destination_transform.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_transform.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_transform.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_y() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_z() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}
