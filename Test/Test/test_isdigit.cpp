#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isdigit_true, "ft_isdigit true")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(1, ft_isdigit('5'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(1, ft_isdigit('0'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(1, ft_isdigit('9'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_isdigit_false, "ft_isdigit false")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isdigit('a'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isdigit('/'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isdigit(-1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_isdigit_extended_ascii, "ft_isdigit rejects extended ASCII")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isdigit(0xC8));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_isdigit(0xFF));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
