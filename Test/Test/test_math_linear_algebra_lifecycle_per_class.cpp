#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

static int math_expect_sigabrt_per_class(void (*operation)())
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

static void vector2_copy_from_uninitialized_source()
{
    vector2 destination_value(1.0, 2.0);
    vector2 source_value;

    (void)destination_value.initialize(source_value);
    return ;
}

static void vector3_copy_from_uninitialized_source()
{
    vector3 destination_value(1.0, 2.0, 3.0);
    vector3 source_value;

    (void)destination_value.initialize(source_value);
    return ;
}

static void vector4_copy_from_uninitialized_source()
{
    vector4 destination_value(1.0, 2.0, 3.0, 4.0);
    vector4 source_value;

    (void)destination_value.initialize(source_value);
    return ;
}

static void matrix2_copy_from_uninitialized_source()
{
    matrix2 destination_value(1.0, 0.0, 0.0, 1.0);
    matrix2 source_value;

    (void)destination_value.initialize(source_value);
    return ;
}

static void matrix3_copy_from_uninitialized_source()
{
    matrix3 destination_value(1.0, 0.0, 0.0,
                              0.0, 1.0, 0.0,
                              0.0, 0.0, 1.0);
    matrix3 source_value;

    (void)destination_value.initialize(source_value);
    return ;
}

static void matrix4_copy_from_uninitialized_source()
{
    matrix4 destination_value(1.0, 0.0, 0.0, 0.0,
                              0.0, 1.0, 0.0, 0.0,
                              0.0, 0.0, 1.0, 0.0,
                              0.0, 0.0, 0.0, 1.0);
    matrix4 source_value;

    (void)destination_value.initialize(source_value);
    return ;
}

static void quaternion_copy_from_uninitialized_source()
{
    quaternion destination_value(1.0, 0.0, 0.0, 0.0);
    quaternion source_value;

    (void)destination_value.initialize(source_value);
    return ;
}

FT_TEST(test_vector2_copy_from_uninitialized_source_aborts,
    "vector2 initialize(copy) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_per_class(vector2_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_vector3_copy_from_uninitialized_source_aborts,
    "vector3 initialize(copy) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_per_class(vector3_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_vector4_copy_from_uninitialized_source_aborts,
    "vector4 initialize(copy) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_per_class(vector4_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_matrix2_copy_from_uninitialized_source_aborts,
    "matrix2 initialize(copy) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_per_class(matrix2_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_matrix3_copy_from_uninitialized_source_aborts,
    "matrix3 initialize(copy) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_per_class(matrix3_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_matrix4_copy_from_uninitialized_source_aborts,
    "matrix4 initialize(copy) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_per_class(matrix4_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_quaternion_copy_from_uninitialized_source_aborts,
    "quaternion initialize(copy) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt_per_class(quaternion_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_vector2_destroy_then_reinitialize,
    "vector2 can initialize again after destroy")
{
    vector2 vector_value(1.0, 2.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.initialize(9.0, 8.0));
    FT_ASSERT(math_fabs(vector_value.get_x() - 9.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y() - 8.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_destroy_then_reinitialize,
    "vector3 can initialize again after destroy")
{
    vector3 vector_value(1.0, 2.0, 3.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.initialize(7.0, 8.0, 9.0));
    FT_ASSERT(math_fabs(vector_value.get_x() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y() - 8.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_z() - 9.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_destroy_then_reinitialize,
    "vector4 can initialize again after destroy")
{
    vector4 vector_value(1.0, 2.0, 3.0, 4.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.initialize(5.0, 6.0, 7.0, 8.0));
    FT_ASSERT(math_fabs(vector_value.get_x() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_z() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_w() - 8.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_destroy_then_reinitialize,
    "matrix2 can initialize again after destroy")
{
    matrix2 matrix_value(1.0, 2.0, 3.0, 4.0);
    vector2 vector_value(1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.initialize(2.0, 0.0, 0.0, 3.0));
    vector2 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_destroy_then_reinitialize,
    "matrix3 can initialize again after destroy")
{
    matrix3 matrix_value(1.0, 2.0, 3.0,
                         4.0, 5.0, 6.0,
                         7.0, 8.0, 9.0);
    vector3 vector_value(1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.initialize(2.0, 0.0, 0.0,
            0.0, 3.0, 0.0, 0.0, 0.0, 4.0));
    vector3 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_destroy_then_reinitialize,
    "matrix4 can initialize again after destroy")
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);
    vector4 vector_value(1.0, 1.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.initialize(2.0, 0.0, 0.0, 0.0,
            0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0));
    vector4 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_w() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_destroy_then_reinitialize,
    "quaternion can initialize again after destroy")
{
    quaternion quaternion_value(1.0, 2.0, 3.0, 4.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.initialize(5.0, 6.0, 7.0, 8.0));
    FT_ASSERT(math_fabs(quaternion_value.get_w() - 5.0) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_x() - 6.0) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_y() - 7.0) < 0.000001);
    FT_ASSERT(math_fabs(quaternion_value.get_z() - 8.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}
