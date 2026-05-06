#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_atol_simple)
{
    FT_ASSERT_EQ(1234L, ft_atol("1234"));
    return (1);
}

FT_TEST(test_basic_atol_negative)
{
    FT_ASSERT_EQ(-9876L, ft_atol("-9876"));
    return (1);
}

FT_TEST(test_basic_atol_with_plus_sign)
{
    FT_ASSERT_EQ(42L, ft_atol("+42"));
    return (1);
}

FT_TEST(test_basic_atol_with_leading_whitespace)
{
    FT_ASSERT_EQ(42L, ft_atol(" \t\n +0042"));
    return (1);
}

FT_TEST(test_basic_atol_stops_at_trailing_characters)
{
    FT_ASSERT_EQ(98765L, ft_atol("98765abc"));
    return (1);
}

FT_TEST(test_basic_atol_null_input)
{
    FT_ASSERT_EQ(0L, ft_atol(ft_nullptr));
    return (1);
}

FT_TEST(test_basic_atol_no_digits)
{
    FT_ASSERT_EQ(0L, ft_atol("abc"));
    return (1);
}
