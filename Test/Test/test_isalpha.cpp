#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isalpha, "ft_isalpha")
{
    FT_ASSERT_EQ(1, ft_isalpha('a'));
    FT_ASSERT_EQ(1, ft_isalpha('Z'));
    FT_ASSERT_EQ(0, ft_isalpha('1'));
    FT_ASSERT_EQ(0, ft_isalpha('/'));
    FT_ASSERT_EQ(0, ft_isalpha(-1));
    return (1);
}
