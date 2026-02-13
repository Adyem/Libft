#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_thread_safety_enable_disable_idempotent,
    "vector2 enable/disable thread safety is idempotent")
{
    vector2 vector_value(1.0, 2.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(true, vector_value.is_thread_safe_enabled());
    vector_value.disable_thread_safety();
    vector_value.disable_thread_safety();
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_thread_safety_enable_disable_idempotent,
    "vector3 enable/disable thread safety is idempotent")
{
    vector3 vector_value(1.0, 2.0, 3.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(true, vector_value.is_thread_safe_enabled());
    vector_value.disable_thread_safety();
    vector_value.disable_thread_safety();
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_thread_safety_enable_disable_idempotent,
    "vector4 enable/disable thread safety is idempotent")
{
    vector4 vector_value(1.0, 2.0, 3.0, 4.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(true, vector_value.is_thread_safe_enabled());
    vector_value.disable_thread_safety();
    vector_value.disable_thread_safety();
    FT_ASSERT_EQ(false, vector_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_thread_safety_enable_disable_idempotent,
    "matrix2 enable/disable thread safety is idempotent")
{
    matrix2 matrix_value(1.0, 0.0, 0.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(true, matrix_value.is_thread_safe_enabled());
    matrix_value.disable_thread_safety();
    matrix_value.disable_thread_safety();
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_thread_safety_enable_disable_idempotent,
    "matrix3 enable/disable thread safety is idempotent")
{
    matrix3 matrix_value(1.0, 0.0, 0.0,
                         0.0, 1.0, 0.0,
                         0.0, 0.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(true, matrix_value.is_thread_safe_enabled());
    matrix_value.disable_thread_safety();
    matrix_value.disable_thread_safety();
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_thread_safety_enable_disable_idempotent,
    "matrix4 enable/disable thread safety is idempotent")
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.enable_thread_safety());
    FT_ASSERT_EQ(true, matrix_value.is_thread_safe_enabled());
    matrix_value.disable_thread_safety();
    matrix_value.disable_thread_safety();
    FT_ASSERT_EQ(false, matrix_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_thread_safety_enable_disable_idempotent,
    "quaternion enable/disable thread safety is idempotent")
{
    quaternion quaternion_value(1.0, 0.0, 0.0, 0.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.enable_thread_safety());
    FT_ASSERT_EQ(true, quaternion_value.is_thread_safe_enabled());
    quaternion_value.disable_thread_safety();
    quaternion_value.disable_thread_safety();
    FT_ASSERT_EQ(false, quaternion_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}
