#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/CMA/CMA.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cpp_class_string_append_reports_allocation_failure)
{
    ft_string string_value;
    int32_t append_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("base"));
    cma_set_alloc_limit(1);
    append_error = string_value.append(" + growth that requires allocation");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, append_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, string_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_string_assign_reports_allocation_failure)
{
    ft_string string_value;
    int32_t assign_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("x"));
    cma_set_alloc_limit(1);
    assign_error = string_value.assign("01234567890123456789", 20);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, assign_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, string_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_string_plus_proxy_failure_converts_to_string)
{
    ft_string left_value;
    ft_string right_value;
    ft_string result_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.initialize("left"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.initialize("right"));
    cma_set_alloc_limit(1);
    result_value = left_value + right_value;
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, result_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result_value.destroy());
    return (1);
}

FT_TEST(test_cpp_class_string_substr_failure_returns_null)
{
    ft_string string_value;
    ft_string *substring;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("01234567890123456789"));
    cma_set_alloc_limit(1);
    substring = string_value.substr(0, 10);
    cma_set_alloc_limit(0);
    FT_ASSERT(substring == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, string_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}
