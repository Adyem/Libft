#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_atol_simple, "ft_atol parses a positive number")
{
    FT_ASSERT_EQ(1234L, ft_atol("1234"));
    return (1);
}

FT_TEST(test_atol_negative, "ft_atol parses a negative number")
{
    FT_ASSERT_EQ(-9876L, ft_atol("-9876"));
    return (1);
}

FT_TEST(test_atol_with_plus_sign, "ft_atol parses plus sign")
{
    FT_ASSERT_EQ(42L, ft_atol("+42"));
    return (1);
}

FT_TEST(test_atol_with_leading_whitespace, "ft_atol skips leading whitespace")
{
    FT_ASSERT_EQ(42L, ft_atol(" \t\n +0042"));
    return (1);
}

FT_TEST(test_atol_stops_at_trailing_characters, "ft_atol stops at first non digit")
{
    FT_ASSERT_EQ(98765L, ft_atol("98765abc"));
    return (1);
}

FT_TEST(test_atol_null_input, "ft_atol returns zero for null input")
{
    FT_ASSERT_EQ(0L, ft_atol(ft_nullptr));
    return (1);
}

FT_TEST(test_atol_no_digits, "ft_atol returns zero when no digits are found")
{
    FT_ASSERT_EQ(0L, ft_atol("abc"));
    return (1);
}
