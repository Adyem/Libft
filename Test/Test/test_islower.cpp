#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_islower, "ft_islower")
{
    FT_ASSERT_EQ(1, ft_islower('a'));
    FT_ASSERT_EQ(1, ft_islower('z'));
    FT_ASSERT_EQ(0, ft_islower('A'));
    FT_ASSERT_EQ(0, ft_islower('0'));
    FT_ASSERT_EQ(0, ft_islower(-1));
    return (1);
}
