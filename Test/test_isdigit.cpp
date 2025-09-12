#include "../Libft/libft.hpp"
#include "../System_utils/test_runner.hpp"

FT_TEST(test_isdigit_true, "ft_isdigit true")
{
    FT_ASSERT_EQ(1, ft_isdigit('5'));
    return (1);
}

FT_TEST(test_isdigit_false, "ft_isdigit false")
{
    FT_ASSERT_EQ(0, ft_isdigit('a'));
    return (1);
}
