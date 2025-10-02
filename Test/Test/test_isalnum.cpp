#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isalnum, "ft_isalnum")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(1, ft_isalnum('a'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(1, ft_isalnum('9'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isalnum('/'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isalnum(-1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_isalnum_extended_ascii, "ft_isalnum rejects extended ASCII")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isalnum(0xC0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isalnum(0xFF));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
