#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isprint, "ft_isprint")
{
    FT_ASSERT_EQ(1, ft_isprint('A'));
    FT_ASSERT_EQ(1, ft_isprint(' '));
    FT_ASSERT_EQ(0, ft_isprint('\n'));
    FT_ASSERT_EQ(0, ft_isprint(127));
    FT_ASSERT_EQ(0, ft_isprint(-1));
    return (1);
}
