#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_isalpha)
{
    FT_ASSERT_EQ(1, ft_isalpha('a'));
    FT_ASSERT_EQ(1, ft_isalpha('Z'));
    FT_ASSERT_EQ(0, ft_isalpha('1'));
    FT_ASSERT_EQ(0, ft_isalpha('/'));
    FT_ASSERT_EQ(0, ft_isalpha(-1));
    return (1);
}

FT_TEST(test_basic_isalpha_high_bit)
{
    FT_ASSERT_EQ(0, ft_isalpha(0xC0));
    FT_ASSERT_EQ(0, ft_isalpha(0xFF));
    return (1);
}

FT_TEST(test_basic_isalpha_clears_errno_after_invalid_flag)
{
    FT_ASSERT_EQ(0, ft_isalpha('4'));
    FT_ASSERT_EQ(1, ft_isalpha('q'));
    return (1);
}
