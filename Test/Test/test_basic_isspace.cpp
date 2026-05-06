#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_isspace)
{
    FT_ASSERT_EQ(1, ft_isspace(' '));
    FT_ASSERT_EQ(1, ft_isspace('\n'));
    FT_ASSERT_EQ(1, ft_isspace('\t'));
    FT_ASSERT_EQ(0, ft_isspace('a'));
    FT_ASSERT_EQ(0, ft_isspace(0));
    return (1);
}

FT_TEST(test_basic_isspace_additional_controls)
{
    FT_ASSERT_EQ(1, ft_isspace('\f'));
    FT_ASSERT_EQ(1, ft_isspace('\r'));
    FT_ASSERT_EQ(1, ft_isspace('\v'));
    FT_ASSERT_EQ(0, ft_isspace('\b'));
    FT_ASSERT_EQ(0, ft_isspace(0xA0));
    return (1);
}
