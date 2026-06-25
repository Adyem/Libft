#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_isprint)
{
    FT_ASSERT_EQ(1, ft_isprint('A'));
    FT_ASSERT_EQ(1, ft_isprint(' '));
    FT_ASSERT_EQ(0, ft_isprint('\n'));
    FT_ASSERT_EQ(0, ft_isprint(127));
    FT_ASSERT_EQ(0, ft_isprint(-1));
    return (1);
}

FT_TEST(test_basic_isprint_extended_ascii)
{
    FT_ASSERT_EQ(0, ft_isprint(0x80));
    FT_ASSERT_EQ(0, ft_isprint(0xA0));
    return (1);
}

FT_TEST(test_basic_isprint_recovers_errno_after_control_input)
{
    FT_ASSERT_EQ(0, ft_isprint('\t'));
    FT_ASSERT_EQ(1, ft_isprint('~'));
    return (1);
}
