#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_islower, "ft_islower")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_islower('a'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_islower('z'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_islower('A'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_islower('0'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_islower(-1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_islower_rejects_adjacent_ascii, "ft_islower rejects punctuation and extended ASCII")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_islower('`'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_islower('{'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_islower(0xE1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_islower(0x5F));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
