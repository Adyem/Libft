#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strcpy_s_basic)
{
    char destination[6];

    destination[0] = 'x';
    destination[1] = 'x';
    destination[2] = 'x';
    destination[3] = 'x';
    destination[4] = 'x';
    destination[5] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcpy_s(destination, sizeof(destination), "hi"));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('i', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_basic_strcpy_s_too_small)
{
    char destination[3];

    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = 'c';
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_strcpy_s(destination, sizeof(destination), "hello"));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_basic_strcpy_s_null_argument)
{
    char destination[2];

    destination[0] = 'x';
    destination[1] = '\0';
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_strcpy_s(ft_nullptr, sizeof(destination), "a"));
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_strcpy_s(destination, sizeof(destination), ft_nullptr));
    return (1);
}

FT_TEST(test_basic_strcat_s_basic)
{
    char destination[9];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    destination[3] = 'x';
    destination[4] = 'x';
    destination[5] = 'x';
    destination[6] = 'x';
    destination[7] = 'x';
    destination[8] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcat_s(destination, sizeof(destination), " there"));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('i', destination[1]);
    FT_ASSERT_EQ(' ', destination[2]);
    FT_ASSERT_EQ('t', destination[3]);
    FT_ASSERT_EQ('h', destination[4]);
    FT_ASSERT_EQ('e', destination[5]);
    FT_ASSERT_EQ('r', destination[6]);
    FT_ASSERT_EQ('e', destination[7]);
    FT_ASSERT_EQ('\0', destination[8]);
    return (1);
}

FT_TEST(test_basic_strcat_s_too_small)
{
    char destination[6];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    destination[3] = 'x';
    destination[4] = 'x';
    destination[5] = '\0';
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_strcat_s(destination, sizeof(destination), " world"));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    FT_ASSERT_EQ('\0', destination[5]);
    return (1);
}

FT_TEST(test_basic_strcat_s_null_argument)
{
    char destination[2];

    destination[0] = 'a';
    destination[1] = '\0';
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_strcat_s(ft_nullptr, sizeof(destination), "a"));
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_strcat_s(destination, sizeof(destination), ft_nullptr));
    return (1);
}

FT_TEST(test_basic_strcat_s_recovers_after_truncation)
{
    char destination[5];

    destination[0] = 'o';
    destination[1] = 'k';
    destination[2] = '\0';
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_strcat_s(destination, sizeof(destination), "world"));
    destination[0] = 'o';
    destination[1] = 'k';
    destination[2] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcat_s(destination, sizeof(destination), "ay"));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "okay"));
    return (1);
}

FT_TEST(test_basic_strcpy_s_recovers_after_truncation)
{
    char destination[6];

    destination[0] = 'x';
    destination[1] = 'x';
    destination[2] = 'x';
    destination[3] = '\0';
    destination[4] = '\0';
    destination[5] = '\0';
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_strcpy_s(destination, 3, "toolong"));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcpy_s(destination, sizeof(destination), "ok"));
    FT_ASSERT_EQ('o', destination[0]);
    FT_ASSERT_EQ('k', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_basic_strcpy_s_empty_source_succeeds)
{
    char destination[4];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = 'q';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcpy_s(destination, sizeof(destination), ""));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    FT_ASSERT_EQ('z', destination[2]);
    FT_ASSERT_EQ('q', destination[3]);
    return (1);
}

FT_TEST(test_basic_strcpy_s_exact_fit_succeeds)
{
    char destination[4];

    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcpy_s(destination, sizeof(destination), "cat"));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "cat"));
    return (1);
}

FT_TEST(test_basic_strcpy_s_zero_destination_size_preserves_buffer)
{
    char destination[4];

    destination[0] = 'k';
    destination[1] = 'e';
    destination[2] = 'e';
    destination[3] = 'p';
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_strcpy_s(destination, 0, "x"));
    FT_ASSERT_EQ('k', destination[0]);
    FT_ASSERT_EQ('e', destination[1]);
    FT_ASSERT_EQ('e', destination[2]);
    FT_ASSERT_EQ('p', destination[3]);
    return (1);
}

FT_TEST(test_basic_strcat_s_append_empty_source_preserves_content)
{
    char destination[8];

    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcpy_s(destination, sizeof(destination), "abc"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcat_s(destination, sizeof(destination), ""));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "abc"));
    return (1);
}

FT_TEST(test_basic_strcat_s_appends_into_empty_destination)
{
    char destination[8];

    destination[0] = '\0';
    destination[1] = 'x';
    destination[2] = 'x';
    destination[3] = 'x';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcat_s(destination, sizeof(destination), "go"));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "go"));
    return (1);
}

FT_TEST(test_basic_strcat_s_null_source_zeroes_destination)
{
    char destination[6];

    destination[0] = 'v';
    destination[1] = 'a';
    destination[2] = 'l';
    destination[3] = 'u';
    destination[4] = 'e';
    destination[5] = '\0';
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_strcat_s(destination,
            sizeof(destination), ft_nullptr));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    FT_ASSERT_EQ('\0', destination[5]);
    return (1);
}

FT_TEST(test_basic_strcat_s_exact_fit_succeeds)
{
    char destination[6];

    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcpy_s(destination, sizeof(destination), "ab"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_strcat_s(destination, sizeof(destination), "cde"));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "abcde"));
    return (1);
}

FT_TEST(test_basic_strcat_s_zero_destination_size_preserves_buffer)
{
    char destination[5];

    destination[0] = 'b';
    destination[1] = 'a';
    destination[2] = 's';
    destination[3] = 'e';
    destination[4] = '\0';
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_strcat_s(destination, 0, "x"));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "base"));
    return (1);
}
