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

FT_TEST(test_strchr_returns_first_match, "ft_strchr returns first occurrence")
{
    const char *string = "mississippi";

    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(string + 2, ft_strchr(string, 's'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_negative_value, "ft_strchr handles negative search values")
{
    char string[4];

    string[0] = 'a';
    string[1] = static_cast<char>(0xFF);
    string[2] = 'b';
    string[3] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(string + 1, ft_strchr(string, -1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_not_found_preserves_errno, "ft_strchr clears errno when character missing")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(ft_nullptr, ft_strchr("sample", 'z'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_terminator_clears_errno, "ft_strchr terminator search resets errno")
{
    const char *string;

    string = "edge";
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(string + 4, ft_strchr(string, '\0'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
