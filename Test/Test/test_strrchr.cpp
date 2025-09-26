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
