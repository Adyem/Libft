#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isdigit_true, "ft_isdigit true")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isdigit('5'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isdigit('0'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isdigit('9'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isdigit_false, "ft_isdigit false")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isdigit('a'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isdigit('/'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isdigit(-1));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isdigit_extended_ascii, "ft_isdigit rejects extended ASCII")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isdigit(0xC8));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isdigit(0xFF));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isdigit_resets_errno_after_invalid_state, "ft_isdigit clears errno after prior error state")
{
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(0, ft_isdigit(' '));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isdigit('2'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
