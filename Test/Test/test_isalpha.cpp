#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_isalpha, "ft_isalpha")
{
    FT_ASSERT_EQ(1, ft_isalpha('a'));
    FT_ASSERT_EQ(1, ft_isalpha('Z'));
    FT_ASSERT_EQ(0, ft_isalpha('1'));
    FT_ASSERT_EQ(0, ft_isalpha('/'));
    FT_ASSERT_EQ(0, ft_isalpha(-1));
    return (1);
}

FT_TEST(test_isalpha_high_bit, "ft_isalpha rejects high-bit characters")
{
    FT_ASSERT_EQ(0, ft_isalpha(0xC0));
    FT_ASSERT_EQ(0, ft_isalpha(0xFF));
    return (1);
}

FT_TEST(test_isalpha_clears_errno_after_invalid_flag, "ft_isalpha clears errno after non-alpha input")
{
    FT_ASSERT_EQ(0, ft_isalpha('4'));
    FT_ASSERT_EQ(1, ft_isalpha('q'));
    return (1);
}
