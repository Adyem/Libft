#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_enable_thread_safety_alloc_failure_once,
    "vector2 enable_thread_safety fails once under CMA limit")
{
    vector2 vector_value(1.0, 2.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector2_enable_thread_safety_alloc_failure_twice,
    "vector2 enable_thread_safety keeps failing under CMA limit")
{
    vector2 vector_value(1.0, 2.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector2_enable_thread_safety_alloc_failure_then_recover,
    "vector2 recovers after CMA allocation failure")
{
    vector2 vector_value(1.0, 2.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(true, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_enable_thread_safety_alloc_failure_once,
    "vector3 enable_thread_safety fails once under CMA limit")
{
    vector3 vector_value(1.0, 2.0, 3.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_enable_thread_safety_alloc_failure_twice,
    "vector3 enable_thread_safety keeps failing under CMA limit")
{
    vector3 vector_value(1.0, 2.0, 3.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_enable_thread_safety_alloc_failure_then_recover,
    "vector3 recovers after CMA allocation failure")
{
    vector3 vector_value(1.0, 2.0, 3.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(true, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_enable_thread_safety_alloc_failure_once,
    "vector4 enable_thread_safety fails once under CMA limit")
{
    vector4 vector_value(1.0, 2.0, 3.0, 4.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_enable_thread_safety_alloc_failure_twice,
    "vector4 enable_thread_safety keeps failing under CMA limit")
{
    vector4 vector_value(1.0, 2.0, 3.0, 4.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_enable_thread_safety_alloc_failure_then_recover,
    "vector4 recovers after CMA allocation failure")
{
    vector4 vector_value(1.0, 2.0, 3.0, 4.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(true, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_enable_thread_safety_alloc_failure_once,
    "matrix2 enable_thread_safety fails once under CMA limit")
{
    matrix2 matrix_value(1.0, 0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_enable_thread_safety_alloc_failure_twice,
    "matrix2 enable_thread_safety keeps failing under CMA limit")
{
    matrix2 matrix_value(1.0, 0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_enable_thread_safety_alloc_failure_then_recover,
    "matrix2 recovers after CMA allocation failure")
{
    matrix2 matrix_value(1.0, 0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(true, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_enable_thread_safety_alloc_failure_once,
    "matrix3 enable_thread_safety fails once under CMA limit")
{
    matrix3 matrix_value(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_enable_thread_safety_alloc_failure_twice,
    "matrix3 enable_thread_safety keeps failing under CMA limit")
{
    matrix3 matrix_value(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_enable_thread_safety_alloc_failure_then_recover,
    "matrix3 recovers after CMA allocation failure")
{
    matrix3 matrix_value(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(true, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_enable_thread_safety_alloc_failure_once,
    "matrix4 enable_thread_safety fails once under CMA limit")
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_enable_thread_safety_alloc_failure_twice,
    "matrix4 enable_thread_safety keeps failing under CMA limit")
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_enable_thread_safety_alloc_failure_then_recover,
    "matrix4 recovers after CMA allocation failure")
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, matrix_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(true, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_enable_thread_safety_alloc_failure_once,
    "quaternion enable_thread_safety fails once under CMA limit")
{
    quaternion quaternion_value(1.0, 0.0, 0.0, 0.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, quaternion_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_enable_thread_safety_alloc_failure_twice,
    "quaternion enable_thread_safety keeps failing under CMA limit")
{
    quaternion quaternion_value(1.0, 0.0, 0.0, 0.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, quaternion_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, quaternion_value.enable_thread_safety());
    FT_ASSERT_EQ(false, quaternion_value.is_thread_safe_enabled());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_enable_thread_safety_alloc_failure_then_recover,
    "quaternion recovers after CMA allocation failure")
{
    quaternion quaternion_value(1.0, 0.0, 0.0, 0.0);

    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, quaternion_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.enable_thread_safety());
    FT_ASSERT_EQ(true, quaternion_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}
