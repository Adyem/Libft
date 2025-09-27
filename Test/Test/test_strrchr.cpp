#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strrchr_last, "ft_strrchr last occurrence")
{
    const char *string = "banana";

    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(string + 5, ft_strrchr(string, 'a'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strrchr_not_found, "ft_strrchr not found")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strrchr("hello", 'x'));
    return (1);
}

FT_TEST(test_strrchr_null, "ft_strrchr nullptr")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_strrchr(ft_nullptr, 'a'));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_strrchr_terminator, "ft_strrchr terminator")
{
    const char *string = "world";

    FT_ASSERT_EQ(string + 5, ft_strrchr(string, '\0'));
    return (1);
}

FT_TEST(test_strrchr_high_bit, "ft_strrchr locates high-bit character")
{
    char string[6];

    string[0] = 'n';
    string[1] = static_cast<char>(0xFF);
    string[2] = 'i';
    string[3] = static_cast<char>(0xFF);
    string[4] = 'l';
    string[5] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(string + 3, ft_strrchr(string, 0xFF));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
