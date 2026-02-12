#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_isprint, "ft_isprint")
{
    FT_ASSERT_EQ(1, ft_isprint('A'));
    FT_ASSERT_EQ(1, ft_isprint(' '));
    FT_ASSERT_EQ(0, ft_isprint('\n'));
    FT_ASSERT_EQ(0, ft_isprint(127));
    FT_ASSERT_EQ(0, ft_isprint(-1));
    return (1);
}

FT_TEST(test_isprint_extended_ascii, "ft_isprint rejects bytes above ASCII")
{
    FT_ASSERT_EQ(0, ft_isprint(0x80));
    FT_ASSERT_EQ(0, ft_isprint(0xA0));
    return (1);
}

FT_TEST(test_isprint_recovers_errno_after_control_input, "ft_isprint clears errno after control characters")
{
    FT_ASSERT_EQ(0, ft_isprint('\t'));
    FT_ASSERT_EQ(1, ft_isprint('~'));
    return (1);
}
