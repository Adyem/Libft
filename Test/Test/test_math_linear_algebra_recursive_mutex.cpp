#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_vector2_recursive_mutex_lock_unlock_paths,
    "vector2 methods keep recursive mutex balanced")
{
    vector2 vector_value(1.0, 2.0);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = vector_value.get_mutex_for_testing();
    FT_ASSERT_NE(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT(math_fabs(vector_value.get_x() - 1.0) < 0.000001);
    FT_ASSERT(math_fabs(vector_value.get_y() - 2.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector3_recursive_mutex_lock_unlock_paths,
    "vector3 methods keep recursive mutex balanced")
{
    vector3 vector_value(1.0, 2.0, 3.0);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = vector_value.get_mutex_for_testing();
    FT_ASSERT_NE(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT(math_fabs(vector_value.length() - math_sqrt(14.0)) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_vector4_recursive_mutex_lock_unlock_paths,
    "vector4 methods keep recursive mutex balanced")
{
    vector4 vector_value(1.0, 2.0, 3.0, 4.0);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = vector_value.get_mutex_for_testing();
    FT_ASSERT_NE(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT(math_fabs(vector_value.dot(vector_value) - 30.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix2_recursive_mutex_lock_unlock_paths,
    "matrix2 methods keep recursive mutex balanced")
{
    matrix2 matrix_value(1.0, 2.0, 3.0, 4.0);
    vector2 vector_value(1.0, 1.0);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = matrix_value.get_mutex_for_testing();
    FT_ASSERT_NE(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    vector2 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 7.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix3_recursive_mutex_lock_unlock_paths,
    "matrix3 methods keep recursive mutex balanced")
{
    matrix3 matrix_value(1.0, 2.0, 3.0,
                         4.0, 5.0, 6.0,
                         7.0, 8.0, 9.0);
    vector3 vector_value(1.0, 0.0, -1.0);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = matrix_value.get_mutex_for_testing();
    FT_ASSERT_NE(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    vector3 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() + 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() + 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() + 2.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_matrix4_recursive_mutex_lock_unlock_paths,
    "matrix4 methods keep recursive mutex balanced")
{
    matrix4 matrix_value(1.0, 0.0, 0.0, 1.0,
                         0.0, 1.0, 0.0, 2.0,
                         0.0, 0.0, 1.0, 3.0,
                         0.0, 0.0, 0.0, 1.0);
    vector4 vector_value(1.0, 1.0, 1.0, 1.0);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = matrix_value.get_mutex_for_testing();
    FT_ASSERT_NE(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    vector4 transformed_value = matrix_value.transform(vector_value);
    FT_ASSERT(math_fabs(transformed_value.get_x() - 2.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_y() - 3.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_z() - 4.0) < 0.000001);
    FT_ASSERT(math_fabs(transformed_value.get_w() - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, matrix_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.destroy());
    return (1);
}

FT_TEST(test_quaternion_recursive_mutex_lock_unlock_paths,
    "quaternion methods keep recursive mutex balanced")
{
    quaternion quaternion_value(1.0, 2.0, 3.0, 4.0);
    pt_recursive_mutex *mutex_pointer;
    double normalized_length;

    mutex_pointer = quaternion_value.get_mutex_for_testing();
    FT_ASSERT_NE(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    quaternion normalized_value = quaternion_value.normalize();
    normalized_length = math_sqrt(normalized_value.get_w() * normalized_value.get_w()
            + normalized_value.get_x() * normalized_value.get_x()
            + normalized_value.get_y() * normalized_value.get_y()
            + normalized_value.get_z() * normalized_value.get_z());
    FT_ASSERT(math_fabs(normalized_length - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quaternion_value.destroy());
    return (1);
}
