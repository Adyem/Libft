#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strchr_basic, "ft_strchr basic")
{
    const char *string = "hello";
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(string + 1, ft_strchr(string, 'e'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_not_found, "ft_strchr not found")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strchr("hello", 'x'));
    return (1);
}

FT_TEST(test_strchr_null, "ft_strchr nullptr")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_strchr(ft_nullptr, 'a'));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_strchr_terminator, "ft_strchr terminator")
{
    const char *string = "hello";
    FT_ASSERT_EQ(string + 5, ft_strchr(string, '\0'));
    return (1);
}
