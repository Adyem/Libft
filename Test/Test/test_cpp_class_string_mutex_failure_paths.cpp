#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cpp_class_string_thread_safety_enable_disable_cycle)
{
    ft_string string_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("abc"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, string_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, string_value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_string_failure_paths_keep_object_usable)
{
    ft_string string_value;
    int32_t append_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("abc"));
    append_error = string_value.append(ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, append_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.append("def"));
    FT_ASSERT_EQ(0, ft_strcmp("abcdef", string_value.c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}
