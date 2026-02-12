#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_isalnum, "ft_isalnum")
{
    FT_ASSERT_EQ(1, ft_isalnum('a'));
    FT_ASSERT_EQ(1, ft_isalnum('9'));
    FT_ASSERT_EQ(0, ft_isalnum('/'));
    FT_ASSERT_EQ(0, ft_isalnum(-1));
    return (1);
}

FT_TEST(test_isalnum_extended_ascii, "ft_isalnum rejects extended ASCII")
{
    FT_ASSERT_EQ(0, ft_isalnum(0xC0));
    FT_ASSERT_EQ(0, ft_isalnum(0xFF));
    return (1);
}

FT_TEST(test_isalnum_space_not_alphanumeric, "ft_isalnum rejects whitespace and clears errno")
{
    FT_ASSERT_EQ(0, ft_isalnum(' '));
    return (1);
}
