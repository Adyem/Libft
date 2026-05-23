#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"

FT_TEST(test_basic_constant_time_equal_reports_equal_buffers)
{
    const char *left;
    const char *right;

    left = "secret-value";
    right = "secret-value";
    FT_ASSERT_EQ(FT_TRUE, ft_constant_time_equal(left, right, 12));
    FT_ASSERT_EQ(FT_TRUE, ft_constant_time_equal(ft_nullptr, ft_nullptr, 0));
    return (1);
}

FT_TEST(test_basic_constant_time_equal_reports_different_buffers)
{
    const char *left;
    const char *right;

    left = "secret-value";
    right = "secret-vAlue";
    FT_ASSERT_EQ(FT_FALSE, ft_constant_time_equal(left, right, 12));
    FT_ASSERT_EQ(FT_FALSE, ft_constant_time_equal(left, ft_nullptr, 12));
    return (1);
}
