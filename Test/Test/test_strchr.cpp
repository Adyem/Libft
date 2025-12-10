#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strchr_basic, "ft_strchr basic")
{
    const char *string = "hello";
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(string + 1, ft_strchr(string, 'e'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_not_found, "ft_strchr not found")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strchr("hello", 'x'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_null, "ft_strchr nullptr")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_strchr(ft_nullptr, 'a'));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strchr_terminator, "ft_strchr terminator")
{
    const char *string = "hello";
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(string + 5, ft_strchr(string, '\0'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_returns_first_match, "ft_strchr returns first occurrence")
{
    const char *string = "mississippi";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
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
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(string + 1, ft_strchr(string, -1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_not_found_resets_errno, "ft_strchr clears errno when character missing")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strchr("sample", 'z'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_terminator_clears_errno, "ft_strchr terminator search resets errno")
{
    const char *string;

    string = "edge";
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(string + 4, ft_strchr(string, '\0'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_recovers_after_null_input, "ft_strchr clears errno after null failure")
{
    const char *string;

    string = "recover";
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_strchr(ft_nullptr, 'r'));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(string + 2, ft_strchr(string, 'c'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strchr_empty_string_returns_null_for_non_terminator, "ft_strchr returns nullptr when searching empty strings")
{
    const char *string;

    string = "";
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strchr(string, 'x'));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
