#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/limits.hpp"

FT_TEST(test_atoi_simple, "ft_atoi simple")
{
    FT_ASSERT_EQ(42, ft_atoi("42"));
    return (1);
}

FT_TEST(test_atoi_negative, "ft_atoi negative")
{
    FT_ASSERT_EQ(-13, ft_atoi("-13"));
    return (1);
}

FT_TEST(test_atoi_intmax, "ft_atoi INT_MAX")
{
    ft_string integer_string;

    integer_string = ft_to_string(FT_INT_MAX);
    FT_ASSERT_EQ(FT_INT_MAX, ft_atoi(integer_string.c_str()));
    return (1);
}

FT_TEST(test_atoi_intmin, "ft_atoi INT_MIN")
{
    ft_string integer_string;

    integer_string = ft_to_string(FT_INT_MIN);
    FT_ASSERT_EQ(FT_INT_MIN, ft_atoi(integer_string.c_str()));
    return (1);
}

FT_TEST(test_atoi_overflow_clamps_to_int_max, "ft_atoi clamps overflow to INT_MAX")
{
    ft_string overflow_string;

    overflow_string = ft_to_string(FT_INT_MAX);
    overflow_string += "9";
    FT_ASSERT_EQ(FT_INT_MAX, ft_atoi(overflow_string.c_str()));
    return (1);
}

FT_TEST(test_atoi_underflow_clamps_to_int_min, "ft_atoi clamps underflow to INT_MIN")
{
    ft_string underflow_string;

    underflow_string = ft_to_string(FT_INT_MIN);
    underflow_string += "9";
    FT_ASSERT_EQ(FT_INT_MIN, ft_atoi(underflow_string.c_str()));
    return (1);
}
FT_TEST(test_atoi_whitespace, "ft_atoi leading and trailing whitespace")
{
    FT_ASSERT_EQ(42, ft_atoi("  \t\n42  "));
    return (1);
}

FT_TEST(test_atoi_trailing_chars, "ft_atoi stops at non-digit")
{
    FT_ASSERT_EQ(123, ft_atoi("123abc"));
    return (1);
}

FT_TEST(test_atoi_plus_sign, "ft_atoi plus sign")
{
    FT_ASSERT_EQ(7, ft_atoi("+7"));
    return (1);
}

FT_TEST(test_atoi_zero, "ft_atoi zero")
{
    FT_ASSERT_EQ(0, ft_atoi("0"));
    return (1);
}
