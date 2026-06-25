#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_atoi_simple)
{
    FT_ASSERT_EQ(42, ft_atoi("42"));
    return (1);
}

FT_TEST(test_basic_atoi_negative)
{
    FT_ASSERT_EQ(-13, ft_atoi("-13"));
    return (1);
}

FT_TEST(test_basic_atoi_with_plus_sign)
{
    FT_ASSERT_EQ(7, ft_atoi("+7"));
    return (1);
}

FT_TEST(test_basic_atoi_with_leading_whitespace)
{
    FT_ASSERT_EQ(77, ft_atoi("   77"));
    return (1);
}

FT_TEST(test_basic_atoi_stops_at_trailing_characters)
{
    FT_ASSERT_EQ(123, ft_atoi("123abc"));
    return (1);
}

FT_TEST(test_basic_atoi_zero)
{
    FT_ASSERT_EQ(0, ft_atoi("0"));
    return (1);
}

FT_TEST(test_basic_atoi_null_input)
{
    FT_ASSERT_EQ(0, ft_atoi(ft_nullptr));
    return (1);
}

FT_TEST(test_basic_atoi_no_digits)
{
    FT_ASSERT_EQ(0, ft_atoi("abc"));
    return (1);
}
