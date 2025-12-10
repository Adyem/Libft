#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_isprint, "ft_isprint")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isprint('A'));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isprint(' '));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isprint('\n'));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isprint(127));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isprint(-1));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isprint_extended_ascii, "ft_isprint rejects bytes above ASCII")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isprint(0x80));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0, ft_isprint(0xA0));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_isprint_recovers_errno_after_control_input, "ft_isprint clears errno after control characters")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_isprint('\t'));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1, ft_isprint('~'));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
