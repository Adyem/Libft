#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cpp_class_string_recursive_mutex_reentrant_usage)
{
    ft_string string_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("x"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.append("y"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.append("z"));
    FT_ASSERT_EQ(0, ft_strcmp("xyz", string_value.c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_string_destroy_cleans_thread_safety_state)
{
    ft_string string_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("value"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, string_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    FT_ASSERT_EQ(FT_FALSE, string_value.is_thread_safe());
    return (1);
}
