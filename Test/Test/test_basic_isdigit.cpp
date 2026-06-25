#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_isdigit_true)
{
    FT_ASSERT_EQ(1, ft_isdigit('5'));
    FT_ASSERT_EQ(1, ft_isdigit('0'));
    FT_ASSERT_EQ(1, ft_isdigit('9'));
    return (1);
}

FT_TEST(test_basic_isdigit_false)
{
    FT_ASSERT_EQ(0, ft_isdigit('a'));
    FT_ASSERT_EQ(0, ft_isdigit('/'));
    FT_ASSERT_EQ(0, ft_isdigit(-1));
    return (1);
}

FT_TEST(test_basic_isdigit_extended_ascii)
{
    FT_ASSERT_EQ(0, ft_isdigit(0xC8));
    FT_ASSERT_EQ(0, ft_isdigit(0xFF));
    return (1);
}

FT_TEST(test_basic_isdigit_resets_errno_after_invalid_state)
{
    FT_ASSERT_EQ(0, ft_isdigit(' '));
    FT_ASSERT_EQ(1, ft_isdigit('2'));
    return (1);
}
