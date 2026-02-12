#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_islower, "ft_islower")
{
    FT_ASSERT_EQ(1, ft_islower('a'));
    FT_ASSERT_EQ(1, ft_islower('z'));
    FT_ASSERT_EQ(0, ft_islower('A'));
    FT_ASSERT_EQ(0, ft_islower('0'));
    FT_ASSERT_EQ(0, ft_islower(-1));
    return (1);
}

FT_TEST(test_islower_rejects_adjacent_ascii, "ft_islower rejects punctuation and extended ASCII")
{
    FT_ASSERT_EQ(0, ft_islower('`'));
    FT_ASSERT_EQ(0, ft_islower('{'));
    FT_ASSERT_EQ(0, ft_islower(0xE1));
    FT_ASSERT_EQ(0, ft_islower(0x5F));
    return (1);
}
