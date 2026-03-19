#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <csignal>

#ifndef LIBFT_TEST_BUILD
#endif

static int math_expect_sigabrt(void (*operation)())
{
    return (test_expect_sigabrt_signal(operation));
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

FT_TEST(test_vector2_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(vector2_call_initialize_twice));
    return (1);
}

FT_TEST(test_vector3_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(vector3_call_initialize_twice));
    return (1);
}

FT_TEST(test_vector4_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(vector4_call_initialize_twice));
    return (1);
}

FT_TEST(test_matrix2_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(matrix2_call_initialize_twice));
    return (1);
}

FT_TEST(test_matrix3_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(matrix3_call_initialize_twice));
    return (1);
}

FT_TEST(test_matrix4_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(matrix4_call_initialize_twice));
    return (1);
}

FT_TEST(test_quaternion_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, math_expect_sigabrt(quaternion_call_initialize_twice));
    return (1);
}
