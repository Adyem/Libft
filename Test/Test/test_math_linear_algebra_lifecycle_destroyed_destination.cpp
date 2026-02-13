#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_initialize_copy_into_destroyed_destination,
    "vector2 initialize(copy) succeeds when destination is destroyed")
{
    vector2 source_value(4.0, 6.0);
    vector2 destination_value(1.0, 2.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 6.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector2_initialize_move_into_destroyed_destination,
    "vector2 initialize(move) succeeds when destination is destroyed")
{
    vector2 source_value(4.0, 6.0);
    vector2 destination_value(1.0, 2.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<vector2 &&>(source_value)));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector3_initialize_copy_into_destroyed_destination,
    "vector3 initialize(copy) succeeds when destination is destroyed")
{
    vector3 source_value(4.0, 6.0, 8.0);
    vector3 destination_value(1.0, 2.0, 3.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 8.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector3_initialize_move_into_destroyed_destination,
    "vector3 initialize(move) succeeds when destination is destroyed")
{
    vector3 source_value(4.0, 6.0, 8.0);
    vector3 destination_value(1.0, 2.0, 3.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<vector3 &&>(source_value)));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 8.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector4_initialize_copy_into_destroyed_destination,
    "vector4 initialize(copy) succeeds when destination is destroyed")
{
    vector4 source_value(4.0, 6.0, 8.0, 10.0);
    vector4 destination_value(1.0, 2.0, 3.0, 4.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 8.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_w() - 10.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector4_initialize_move_into_destroyed_destination,
    "vector4 initialize(move) succeeds when destination is destroyed")
{
    vector4 source_value(4.0, 6.0, 8.0, 10.0);
    vector4 destination_value(1.0, 2.0, 3.0, 4.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<vector4 &&>(source_value)));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 8.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_w() - 10.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_z()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_w()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_initialize_copy_into_destroyed_destination,
    "matrix2 initialize(copy) succeeds when destination is destroyed")
{
    matrix2 source_value(2.0, 0.0, 0.0, 3.0);
    matrix2 destination_value(1.0, 0.0, 0.0, 1.0);
    vector2 vector_value(1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    vector2 transformed_value = destination_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_initialize_move_into_destroyed_destination,
    "matrix2 initialize(move) succeeds when destination is destroyed")
{
    matrix2 source_value(2.0, 0.0, 0.0, 3.0);
    matrix2 destination_value(1.0, 0.0, 0.0, 1.0);
    vector2 vector_value(1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
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

FT_TEST(test_matrix3_initialize_copy_into_destroyed_destination,
    "matrix3 initialize(copy) succeeds when destination is destroyed")
{
    matrix3 source_value(2.0, 0.0, 0.0,
                         0.0, 3.0, 0.0,
                         0.0, 0.0, 4.0);
    matrix3 destination_value(1.0, 0.0, 0.0,
                              0.0, 1.0, 0.0,
                              0.0, 0.0, 1.0);
    vector3 vector_value(1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
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

FT_TEST(test_matrix3_initialize_move_into_destroyed_destination,
    "matrix3 initialize(move) succeeds when destination is destroyed")
{
    matrix3 source_value(2.0, 0.0, 0.0,
                         0.0, 3.0, 0.0,
                         0.0, 0.0, 4.0);
    matrix3 destination_value(1.0, 0.0, 0.0,
                              0.0, 1.0, 0.0,
                              0.0, 0.0, 1.0);
    vector3 vector_value(1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
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

FT_TEST(test_matrix4_initialize_copy_into_destroyed_destination,
    "matrix4 initialize(copy) succeeds when destination is destroyed")
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

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
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

FT_TEST(test_matrix4_initialize_move_into_destroyed_destination,
    "matrix4 initialize(move) succeeds when destination is destroyed")
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

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<matrix4 &&>(source_value)));
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

FT_TEST(test_quaternion_initialize_copy_into_destroyed_destination,
    "quaternion initialize(copy) succeeds when destination is destroyed")
{
    quaternion source_value(2.0, 3.0, 5.0, 7.0);
    quaternion destination_value(1.0, 0.0, 0.0, 0.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(destination_value.get_w() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_x() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 7.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_initialize_move_into_destroyed_destination,
    "quaternion initialize(move) succeeds when destination is destroyed")
{
    quaternion source_value(2.0, 3.0, 5.0, 7.0);
    quaternion destination_value(1.0, 0.0, 0.0, 0.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<quaternion &&>(source_value)));
    FT_ASSERT(math_fabs(destination_value.get_w() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_x() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_w() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}
