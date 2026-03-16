#include "../test_internal.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_vector_thread_safety_enable_disable)
{
    ft_vector<int32_t> vector_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.initialize());
    FT_ASSERT_EQ(FT_FALSE, vector_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, vector_value.is_thread_safe());
    vector_value.push_back(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.get_error());
    vector_value.push_back(11);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.get_error());
    FT_ASSERT_EQ(2, vector_value.size());
    FT_ASSERT_EQ(5, vector_value[0]);
    FT_ASSERT_EQ(11, vector_value[1]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, vector_value.is_thread_safe());
    return (1);
}
