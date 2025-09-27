#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isdigit_true, "ft_isdigit true")
{
    FT_ASSERT_EQ(1, ft_isdigit('5'));
    FT_ASSERT_EQ(1, ft_isdigit('0'));
    FT_ASSERT_EQ(1, ft_isdigit('9'));
    return (1);
}

FT_TEST(test_isdigit_false, "ft_isdigit false")
{
    FT_ASSERT_EQ(0, ft_isdigit('a'));
    FT_ASSERT_EQ(0, ft_isdigit('/'));
    FT_ASSERT_EQ(0, ft_isdigit(-1));
    return (1);
}

FT_TEST(test_isdigit_extended_ascii, "ft_isdigit rejects extended ASCII")
{
    FT_ASSERT_EQ(0, ft_isdigit(0xC8));
    FT_ASSERT_EQ(0, ft_isdigit(0xFF));
    return (1);
}
