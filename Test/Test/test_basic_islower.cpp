#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_islower)
{
    FT_ASSERT_EQ(1, ft_islower('a'));
    FT_ASSERT_EQ(1, ft_islower('z'));
    FT_ASSERT_EQ(0, ft_islower('A'));
    FT_ASSERT_EQ(0, ft_islower('0'));
    FT_ASSERT_EQ(0, ft_islower(-1));
    return (1);
}

FT_TEST(test_basic_islower_rejects_adjacent_ascii)
{
    FT_ASSERT_EQ(0, ft_islower('`'));
    FT_ASSERT_EQ(0, ft_islower('{'));
    FT_ASSERT_EQ(0, ft_islower(0xE1));
    FT_ASSERT_EQ(0, ft_islower(0x5F));
    return (1);
}
