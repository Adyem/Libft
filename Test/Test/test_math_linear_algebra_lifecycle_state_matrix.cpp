#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Math/math_interval.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int math_expect_sigabrt_state_matrix(void (*operation)())
{
    return (test_expect_sigabrt_signal(operation));
}

static void vector2_destroyed_get_x_aborts()
{
    vector2 vector_value(1.0, 2.0);

    (void)vector_value.destroy();
    (void)vector_value.get_x();
    return ;
}

static void vector3_destroyed_length_aborts()
{
    vector3 vector_value(1.0, 2.0, 3.0);

    (void)vector_value.destroy();
    (void)vector_value.length();
    return ;
}

static void vector4_destroyed_dot_aborts()
{
    vector4 left_value(1.0, 2.0, 3.0, 4.0);
    vector4 right_value(5.0, 6.0, 7.0, 8.0);

    (void)left_value.destroy();
    (void)left_value.dot(right_value);
    return ;
}

static void matrix2_destroyed_transform_aborts()
{
    matrix2 matrix_value(1.0, 0.0, 0.0, 1.0);
    vector2 vector_value(1.0, 2.0);

    (void)matrix_value.destroy();
    (void)matrix_value.transform(vector_value);
    return ;
}

static void matrix3_destroyed_invert_aborts()
{
    matrix3 matrix_value(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         0.0, 0.0, 1.0);

    (void)matrix_value.destroy();
    (void)matrix_value.invert();
    return ;
}

static void matrix4_destroyed_multiply_aborts()
{
    matrix4 left_value(1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
    matrix4 right_value(2.0, 0.0, 0.0, 0.0,
                        0.0, 2.0, 0.0, 0.0,
                        0.0, 0.0, 2.0, 0.0,
                        0.0, 0.0, 0.0, 1.0);

    (void)left_value.destroy();
    (void)left_value.multiply(right_value);
    return ;
}

static void quaternion_destroyed_normalize_aborts()
{
    quaternion quaternion_value(1.0, 2.0, 3.0, 4.0);

    (void)quaternion_value.destroy();
    (void)quaternion_value.normalize();
    return ;
}

FT_TEST(test_vector2_destroyed_runtime_method_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector2_destroyed_get_x_aborts));
    return (1);
}

FT_TEST(test_vector3_destroyed_runtime_method_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector3_destroyed_length_aborts));
    return (1);
}

FT_TEST(test_vector4_destroyed_runtime_method_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector4_destroyed_dot_aborts));
    return (1);
}

FT_TEST(test_matrix2_destroyed_runtime_method_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix2_destroyed_transform_aborts));
    return (1);
}

FT_TEST(test_matrix3_destroyed_runtime_method_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix3_destroyed_invert_aborts));
    return (1);
}

FT_TEST(test_matrix4_destroyed_runtime_method_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix4_destroyed_multiply_aborts));
    return (1);
}

FT_TEST(test_quaternion_destroyed_runtime_method_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(quaternion_destroyed_normalize_aborts));
    return (1);
}

static void vector2_copy_from_destroyed_source_succeeds()
{
    vector2 source_value(1.0, 2.0);
    vector2 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(source_value);
    return ;
}

static void vector3_move_from_destroyed_source_succeeds()
{
    vector3 source_value(1.0, 2.0, 3.0);
    vector3 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(static_cast<vector3 &&>(source_value));
    return ;
}

static void vector4_copy_from_destroyed_source_succeeds()
{
    vector4 source_value(1.0, 2.0, 3.0, 4.0);
    vector4 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(source_value);
    return ;
}

static void matrix2_move_from_destroyed_source_succeeds()
{
    matrix2 source_value(1.0, 0.0, 0.0, 1.0);
    matrix2 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(static_cast<matrix2 &&>(source_value));
    return ;
}

static void matrix3_copy_from_destroyed_source_succeeds()
{
    matrix3 source_value(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         0.0, 0.0, 1.0);
    matrix3 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(source_value);
    return ;
}

static void matrix4_move_from_destroyed_source_succeeds()
{
    matrix4 source_value(1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);
    matrix4 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(static_cast<matrix4 &&>(source_value));
    return ;
}

static void quaternion_copy_from_destroyed_source_succeeds()
{
    quaternion source_value(1.0, 0.0, 0.0, 0.0);
    quaternion destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(source_value);
    return ;
}

FT_TEST(test_vector2_initialize_copy_from_destroyed_source_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_state_matrix(vector2_copy_from_destroyed_source_succeeds));
    return (1);
}

FT_TEST(test_vector3_initialize_move_from_destroyed_source_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_state_matrix(vector3_move_from_destroyed_source_succeeds));
    return (1);
}

FT_TEST(test_vector4_initialize_copy_from_destroyed_source_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_state_matrix(vector4_copy_from_destroyed_source_succeeds));
    return (1);
}

FT_TEST(test_matrix2_initialize_move_from_destroyed_source_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_state_matrix(matrix2_move_from_destroyed_source_succeeds));
    return (1);
}

FT_TEST(test_matrix3_initialize_copy_from_destroyed_source_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_state_matrix(matrix3_copy_from_destroyed_source_succeeds));
    return (1);
}

FT_TEST(test_matrix4_initialize_move_from_destroyed_source_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_state_matrix(matrix4_move_from_destroyed_source_succeeds));
    return (1);
}

FT_TEST(test_quaternion_initialize_copy_from_destroyed_source_succeeds)
{
    FT_ASSERT_EQ(0, math_expect_sigabrt_state_matrix(quaternion_copy_from_destroyed_source_succeeds));
    return (1);
}
