#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strrchr_last)
{
    const char *string = "banana";

    FT_ASSERT_EQ(string + 5, ft_strrchr(string, 'a'));
    return (1);
}

FT_TEST(test_basic_strrchr_not_found)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strrchr("hello", 'x'));
    return (1);
}

FT_TEST(test_basic_strrchr_null)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strrchr(ft_nullptr, 'a'));
    return (1);
}

FT_TEST(test_basic_strrchr_terminator)
{
    const char *string = "world";

    FT_ASSERT_EQ(string + 5, ft_strrchr(string, '\0'));
    return (1);
}

FT_TEST(test_basic_strrchr_high_bit)
{
    char string[6];

    string[0] = 'n';
    string[1] = static_cast<char>(0xFF);
    string[2] = 'i';
    string[3] = static_cast<char>(0xFF);
    string[4] = 'l';
    string[5] = '\0';
    FT_ASSERT_EQ(string + 3, ft_strrchr(string, 0xFF));
    return (1);
}

FT_TEST(test_basic_strrchr_recovers_after_null_failure)
{
    const char *string = "resume";

    FT_ASSERT_EQ(ft_nullptr, ft_strrchr(ft_nullptr, 'e'));
    FT_ASSERT_EQ(string + 5, ft_strrchr(string, 'e'));
    return (1);
}
