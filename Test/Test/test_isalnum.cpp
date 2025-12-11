#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isalnum, "ft_isalnum")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isalnum('a'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isalnum('9'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isalnum('/'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isalnum(-1));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isalnum_extended_ascii, "ft_isalnum rejects extended ASCII")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isalnum(0xC0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isalnum(0xFF));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isalnum_space_not_alphanumeric, "ft_isalnum rejects whitespace and clears errno")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_isalnum(' '));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
