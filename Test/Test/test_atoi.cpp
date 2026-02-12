#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_atoi_simple, "ft_atoi parses a positive number")
{
    FT_ASSERT_EQ(42, ft_atoi("42"));
    return (1);
}

FT_TEST(test_atoi_negative, "ft_atoi parses a negative number")
{
    FT_ASSERT_EQ(-13, ft_atoi("-13"));
    return (1);
}

FT_TEST(test_atoi_with_plus_sign, "ft_atoi parses plus sign")
{
    FT_ASSERT_EQ(7, ft_atoi("+7"));
    return (1);
}

FT_TEST(test_atoi_with_leading_whitespace, "ft_atoi skips leading whitespace")
{
    FT_ASSERT_EQ(77, ft_atoi("   77"));
    return (1);
}

FT_TEST(test_atoi_stops_at_trailing_characters, "ft_atoi stops at first non digit")
{
    FT_ASSERT_EQ(123, ft_atoi("123abc"));
    return (1);
}

FT_TEST(test_atoi_zero, "ft_atoi parses zero")
{
    FT_ASSERT_EQ(0, ft_atoi("0"));
    return (1);
}

FT_TEST(test_atoi_null_input, "ft_atoi returns zero for null input")
{
    FT_ASSERT_EQ(0, ft_atoi(ft_nullptr));
    return (1);
}

FT_TEST(test_atoi_no_digits, "ft_atoi returns zero when no digits are found")
{
    FT_ASSERT_EQ(0, ft_atoi("abc"));
    return (1);
}
