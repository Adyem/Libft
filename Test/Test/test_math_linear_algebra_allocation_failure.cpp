#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_enable_thread_safety_allocation_failure,
    "vector2 enable_thread_safety reports no memory on CMA allocation failure")
{
    vector2 vector_value(1.0, 2.0);
    int enable_error;

    cma_set_alloc_limit(1);
    enable_error = vector_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_enable_thread_safety_allocation_failure,
    "vector3 enable_thread_safety reports no memory on CMA allocation failure")
{
    vector3 vector_value(1.0, 2.0, 3.0);
    int enable_error;

    cma_set_alloc_limit(1);
    enable_error = vector_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_enable_thread_safety_allocation_failure,
    "vector4 enable_thread_safety reports no memory on CMA allocation failure")
{
    vector4 vector_value(1.0, 2.0, 3.0, 4.0);
    int enable_error;

    cma_set_alloc_limit(1);
    enable_error = vector_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_enable_thread_safety_allocation_failure,
    "matrix2 enable_thread_safety reports no memory on CMA allocation failure")
{
    matrix2 matrix_value(1.0, 0.0, 0.0, 1.0);
    int enable_error;

    cma_set_alloc_limit(1);
    enable_error = matrix_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_enable_thread_safety_allocation_failure,
    "matrix3 enable_thread_safety reports no memory on CMA allocation failure")
{
    matrix3 matrix_value(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         0.0, 0.0, 1.0);
    int enable_error;

    cma_set_alloc_limit(1);
    enable_error = matrix_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_enable_thread_safety_allocation_failure,
    "matrix4 enable_thread_safety reports no memory on CMA allocation failure")
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);
    int enable_error;

    cma_set_alloc_limit(1);
    enable_error = matrix_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_enable_thread_safety_allocation_failure,
    "quaternion enable_thread_safety reports no memory on CMA allocation failure")
{
    quaternion quaternion_value(1.0, 0.0, 0.0, 0.0);
    int enable_error;

    cma_set_alloc_limit(1);
    enable_error = quaternion_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(false, quaternion_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}
