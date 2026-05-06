#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strchr_basic)
{
    const char *string = "hello";
    FT_ASSERT_EQ(string + 1, ft_strchr(string, 'e'));
    return (1);
}

FT_TEST(test_basic_strchr_not_found)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strchr("hello", 'x'));
    return (1);
}

FT_TEST(test_basic_strchr_null)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strchr(ft_nullptr, 'a'));
    return (1);
}

FT_TEST(test_basic_strchr_terminator)
{
    const char *string = "hello";
    FT_ASSERT_EQ(string + 5, ft_strchr(string, '\0'));
    return (1);
}

FT_TEST(test_basic_strchr_returns_first_match)
{
    const char *string = "mississippi";

    FT_ASSERT_EQ(string + 2, ft_strchr(string, 's'));
    return (1);
}

FT_TEST(test_basic_strchr_negative_value)
{
    char string[4];

    string[0] = 'a';
    string[1] = static_cast<char>(0xFF);
    string[2] = 'b';
    string[3] = '\0';
    FT_ASSERT_EQ(string + 1, ft_strchr(string, -1));
    return (1);
}

FT_TEST(test_basic_strchr_not_found_resets_errno)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strchr("sample", 'z'));
    return (1);
}

FT_TEST(test_basic_strchr_terminator_clears_errno)
{
    const char *string;

    string = "edge";
    FT_ASSERT_EQ(string + 4, ft_strchr(string, '\0'));
    return (1);
}

FT_TEST(test_basic_strchr_recovers_after_null_input)
{
    const char *string;

    string = "recover";
    FT_ASSERT_EQ(ft_nullptr, ft_strchr(ft_nullptr, 'r'));
    FT_ASSERT_EQ(string + 2, ft_strchr(string, 'c'));
    return (1);
}

FT_TEST(test_basic_strchr_empty_string_returns_null_for_non_terminator)
{
    const char *string;

    string = "";
    FT_ASSERT_EQ(ft_nullptr, ft_strchr(string, 'x'));
    return (1);
}
