#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isupper, "ft_isupper")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isupper('A'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isupper('Z'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isupper('a'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isupper('1'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isupper(-1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_isupper_rejects_adjacent_ascii, "ft_isupper rejects punctuation and extended ASCII")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isupper('@'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isupper('['));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isupper(0xC0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isupper(0x7B));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
