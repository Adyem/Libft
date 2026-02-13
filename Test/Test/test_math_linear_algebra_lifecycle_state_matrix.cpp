#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

static int math_expect_sigabrt_state_matrix(void (*operation)())
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        operation();
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (!WIFSIGNALED(child_status))
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
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

FT_TEST(test_vector2_destroyed_runtime_method_aborts,
    "vector2 runtime method aborts when called after destroy")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector2_destroyed_get_x_aborts));
    return (1);
}

FT_TEST(test_vector3_destroyed_runtime_method_aborts,
    "vector3 runtime method aborts when called after destroy")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector3_destroyed_length_aborts));
    return (1);
}

FT_TEST(test_vector4_destroyed_runtime_method_aborts,
    "vector4 runtime method aborts when called after destroy")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector4_destroyed_dot_aborts));
    return (1);
}

FT_TEST(test_matrix2_destroyed_runtime_method_aborts,
    "matrix2 runtime method aborts when called after destroy")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix2_destroyed_transform_aborts));
    return (1);
}

FT_TEST(test_matrix3_destroyed_runtime_method_aborts,
    "matrix3 runtime method aborts when called after destroy")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix3_destroyed_invert_aborts));
    return (1);
}

FT_TEST(test_matrix4_destroyed_runtime_method_aborts,
    "matrix4 runtime method aborts when called after destroy")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix4_destroyed_multiply_aborts));
    return (1);
}

FT_TEST(test_quaternion_destroyed_runtime_method_aborts,
    "quaternion runtime method aborts when called after destroy")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(quaternion_destroyed_normalize_aborts));
    return (1);
}

static void vector2_copy_from_destroyed_source_aborts()
{
    vector2 source_value(1.0, 2.0);
    vector2 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(source_value);
    return ;
}

static void vector3_move_from_destroyed_source_aborts()
{
    vector3 source_value(1.0, 2.0, 3.0);
    vector3 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(static_cast<vector3 &&>(source_value));
    return ;
}

static void vector4_copy_from_destroyed_source_aborts()
{
    vector4 source_value(1.0, 2.0, 3.0, 4.0);
    vector4 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(source_value);
    return ;
}

static void matrix2_move_from_destroyed_source_aborts()
{
    matrix2 source_value(1.0, 0.0, 0.0, 1.0);
    matrix2 destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(static_cast<matrix2 &&>(source_value));
    return ;
}

static void matrix3_copy_from_destroyed_source_aborts()
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

static void matrix4_move_from_destroyed_source_aborts()
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

static void quaternion_copy_from_destroyed_source_aborts()
{
    quaternion source_value(1.0, 0.0, 0.0, 0.0);
    quaternion destination_value;

    if (source_value.destroy() != FT_ERR_SUCCESS)
        return ;
    (void)destination_value.initialize(source_value);
    return ;
}

FT_TEST(test_vector2_initialize_copy_from_destroyed_source_aborts,
    "vector2 initialize(copy) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector2_copy_from_destroyed_source_aborts));
    return (1);
}

FT_TEST(test_vector3_initialize_move_from_destroyed_source_aborts,
    "vector3 initialize(move) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector3_move_from_destroyed_source_aborts));
    return (1);
}

FT_TEST(test_vector4_initialize_copy_from_destroyed_source_aborts,
    "vector4 initialize(copy) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(vector4_copy_from_destroyed_source_aborts));
    return (1);
}

FT_TEST(test_matrix2_initialize_move_from_destroyed_source_aborts,
    "matrix2 initialize(move) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix2_move_from_destroyed_source_aborts));
    return (1);
}

FT_TEST(test_matrix3_initialize_copy_from_destroyed_source_aborts,
    "matrix3 initialize(copy) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix3_copy_from_destroyed_source_aborts));
    return (1);
}

FT_TEST(test_matrix4_initialize_move_from_destroyed_source_aborts,
    "matrix4 initialize(move) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(matrix4_move_from_destroyed_source_aborts));
    return (1);
}

FT_TEST(test_quaternion_initialize_copy_from_destroyed_source_aborts,
    "quaternion initialize(copy) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_state_matrix(quaternion_copy_from_destroyed_source_aborts));
    return (1);
}
