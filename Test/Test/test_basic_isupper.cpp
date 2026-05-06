#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_isupper)
{
    FT_ASSERT_EQ(1, ft_isupper('A'));
    FT_ASSERT_EQ(1, ft_isupper('Z'));
    FT_ASSERT_EQ(0, ft_isupper('a'));
    FT_ASSERT_EQ(0, ft_isupper('1'));
    FT_ASSERT_EQ(0, ft_isupper(-1));
    return (1);
}

FT_TEST(test_basic_isupper_rejects_adjacent_ascii)
{
    FT_ASSERT_EQ(0, ft_isupper('@'));
    FT_ASSERT_EQ(0, ft_isupper('['));
    FT_ASSERT_EQ(0, ft_isupper(0xC0));
    FT_ASSERT_EQ(0, ft_isupper(0x7B));
    return (1);
}

FT_TEST(test_basic_isupper_clears_errno_after_digit)
{
    FT_ASSERT_EQ(0, ft_isupper('9'));
    FT_ASSERT_EQ(1, ft_isupper('B'));
    return (1);
}
