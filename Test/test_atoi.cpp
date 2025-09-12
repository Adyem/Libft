#include "../Libft/libft.hpp"
#include "../System_utils/test_runner.hpp"
#include <climits>
#include <string>

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
    std::string integer_string;

    integer_string = std::to_string(INT_MAX);
    FT_ASSERT_EQ(INT_MAX, ft_atoi(integer_string.c_str()));
    return (1);
}

FT_TEST(test_atoi_intmin, "ft_atoi INT_MIN")
{
    std::string integer_string;

    integer_string = std::to_string(INT_MIN);
    FT_ASSERT_EQ(INT_MIN, ft_atoi(integer_string.c_str()));
    return (1);
}
