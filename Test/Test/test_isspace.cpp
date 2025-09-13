#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isspace, "ft_isspace")
{
    FT_ASSERT_EQ(1, ft_isspace(' '));
    FT_ASSERT_EQ(1, ft_isspace('\n'));
    FT_ASSERT_EQ(1, ft_isspace('\t'));
    FT_ASSERT_EQ(0, ft_isspace('a'));
    FT_ASSERT_EQ(0, ft_isspace(0));
    return (1);
}
