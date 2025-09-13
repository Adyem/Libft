#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_time_ms_basic, "ft_time_ms increasing")
{
    long first;
    long second;

    first = ft_time_ms();
    second = ft_time_ms();
    FT_ASSERT(first >= 0);
    FT_ASSERT(second >= first);
    return (1);
}

FT_TEST(test_time_format_basic, "ft_time_format basic")
{
    char buffer[32];

    FT_ASSERT(ft_time_format(buffer, sizeof(buffer)) != ft_nullptr);
    return (1);
}

FT_TEST(test_time_format_errors, "ft_time_format edge cases")
{
    char buffer[32];

    FT_ASSERT_EQ(ft_nullptr, ft_time_format(ft_nullptr, sizeof(buffer)));
    FT_ASSERT_EQ(ft_nullptr, ft_time_format(buffer, 0));
    return (1);
}
