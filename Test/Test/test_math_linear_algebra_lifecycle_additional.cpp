#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static int math_expect_sigabrt_additional(void (*operation)())
{
    return (test_expect_sigabrt_signal(operation));
}

static void vector2_call_destroy_twice()
{
    vector2 vector_value(1.0, 2.0);

    (void)vector_value.destroy();
    (void)vector_value.destroy();
    return ;
}

static void matrix3_call_destroy_while_uninitialised()
{
    matrix3 matrix_value;

    (void)matrix_value.destroy();
    return ;
}

static void quaternion_call_self_move_uninitialised()
{
    alignas(quaternion) unsigned char storage[sizeof(quaternion)];
    quaternion *quaternion_pointer;

    std::memset(storage, 0, sizeof(storage));
    quaternion_pointer = reinterpret_cast<quaternion *>(storage);
    (void)quaternion_pointer->move(*quaternion_pointer);
    return ;
}

FT_TEST(test_vector2_copy_initialize_and_reinitialize_after_destroy)
{
    vector2 source_value(4.0, -2.0);
    vector2 destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() + 2.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(8.0, 3.0));
    FT_ASSERT(math_fabs(destination_value.get_x() - 8.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector3_move_initialize_resets_source)
{
    vector3 source_value(5.0, 6.0, 7.0);
    vector3 destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<vector3 &&>(source_value)));
    FT_ASSERT(math_fabs(destination_value.get_x() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_z()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_vector4_move_method_resets_source)
{
    vector4 source_value(9.0, 8.0, 7.0, 6.0);
    vector4 destination_value(1.0, 1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    FT_ASSERT(math_fabs(destination_value.get_x() - 9.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_y() - 8.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_z() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(destination_value.get_w() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_y()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_z()) < 0.000001);
    FT_ASSERT(math_fabs(source_value.get_w()) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_copy_initialize_keeps_values)
{
    matrix2 source_value(1.0, 2.0, 3.0, 4.0);
    matrix2 destination_value;
    vector2 vector_value(2.0, -1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    vector2 transformed_value = destination_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x()) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 2.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_move_method_resets_source_to_identity)
{
    matrix3 source_value(2.0, 0.0, 0.0,
                         0.0, 3.0, 0.0,
                         0.0, 0.0, 4.0);
    matrix3 destination_value(1.0, 0.0, 0.0,
                              0.0, 1.0, 0.0,
                              0.0, 0.0, 1.0);
    vector3 vector_value(1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    vector3 moved_transform = destination_value.transform(vector_value);
    vector3 source_transform = source_value.transform(vector_value);
    FT_ASSERT(math_fabs(moved_transform.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(moved_transform.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(moved_transform.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_y() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_z() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_move_initialize_resets_source_to_identity)
{
    matrix4 source_value(2.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0, 4.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);
    matrix4 destination_value;
    vector4 vector_value(1.0, 1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<matrix4 &&>(source_value)));
    vector4 moved_transform = destination_value.transform(vector_value);
    vector4 source_transform = source_value.transform(vector_value);
    FT_ASSERT(math_fabs(moved_transform.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(moved_transform.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(moved_transform.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_y() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(source_transform.get_z() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_move_method_resets_source)
{
    quaternion source_value(2.0, 3.0, 4.0, 5.0);
    quaternion destination_value(1.0, 0.0, 0.0, 0.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
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

FT_TEST(test_vector2_destroy_twice_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_additional(vector2_call_destroy_twice));
    return (1);
}

FT_TEST(test_matrix3_destroy_uninitialised_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_additional(matrix3_call_destroy_while_uninitialised));
    return (1);
}

FT_TEST(test_quaternion_self_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_additional(quaternion_call_self_move_uninitialised));
    return (1);
}
