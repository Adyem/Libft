#include "../../Basic/basic.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isupper, "ft_isupper")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isupper('A'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isupper('Z'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isupper('a'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isupper('1'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isupper(-1));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isupper_rejects_adjacent_ascii, "ft_isupper rejects punctuation and extended ASCII")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isupper('@'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isupper('['));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isupper(0xC0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isupper(0x7B));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isupper_clears_errno_after_digit, "ft_isupper resets errno after non-letter input")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_isupper('9'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isupper('B'));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
