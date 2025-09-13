#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isupper, "ft_isupper")
{
    FT_ASSERT_EQ(1, ft_isupper('A'));
    FT_ASSERT_EQ(1, ft_isupper('Z'));
    FT_ASSERT_EQ(0, ft_isupper('a'));
    FT_ASSERT_EQ(0, ft_isupper('1'));
    FT_ASSERT_EQ(0, ft_isupper(-1));
    return (1);
}
