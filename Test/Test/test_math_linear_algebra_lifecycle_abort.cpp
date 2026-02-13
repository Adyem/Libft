#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

static int math_expect_sigabrt(void (*operation)())
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

static void vector2_call_initialize_twice()
{
    vector2 vector_value(1.0, 2.0);

    (void)vector_value.initialize();
    return ;
}

static void vector3_call_initialize_twice()
{
    vector3 vector_value(1.0, 2.0, 3.0);

    (void)vector_value.initialize();
    return ;
}

static void vector4_call_initialize_twice()
{
    vector4 vector_value(1.0, 2.0, 3.0, 4.0);

    (void)vector_value.initialize();
    return ;
}

static void matrix2_call_initialize_twice()
{
    matrix2 matrix_value(1.0, 0.0, 0.0, 1.0);

    (void)matrix_value.initialize();
    return ;
}

static void matrix3_call_initialize_twice()
{
    matrix3 matrix_value(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         0.0, 0.0, 1.0);

    (void)matrix_value.initialize();
    return ;
}

static void matrix4_call_initialize_twice()
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);

    (void)matrix_value.initialize();
    return ;
}

static void quaternion_call_initialize_twice()
{
    quaternion quaternion_value(1.0, 0.0, 0.0, 0.0);

    (void)quaternion_value.initialize();
    return ;
}

FT_TEST(test_vector2_initialize_twice_aborts,
    "vector2 initialize aborts when called on initialized object")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(vector2_call_initialize_twice));
    return (1);
}

FT_TEST(test_vector3_initialize_twice_aborts,
    "vector3 initialize aborts when called on initialized object")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(vector3_call_initialize_twice));
    return (1);
}

FT_TEST(test_vector4_initialize_twice_aborts,
    "vector4 initialize aborts when called on initialized object")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(vector4_call_initialize_twice));
    return (1);
}

FT_TEST(test_matrix2_initialize_twice_aborts,
    "matrix2 initialize aborts when called on initialized object")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(matrix2_call_initialize_twice));
    return (1);
}

FT_TEST(test_matrix3_initialize_twice_aborts,
    "matrix3 initialize aborts when called on initialized object")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(matrix3_call_initialize_twice));
    return (1);
}

FT_TEST(test_matrix4_initialize_twice_aborts,
    "matrix4 initialize aborts when called on initialized object")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(matrix4_call_initialize_twice));
    return (1);
}

FT_TEST(test_quaternion_initialize_twice_aborts,
    "quaternion initialize aborts when called on initialized object")
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(quaternion_call_initialize_twice));
    return (1);
}
