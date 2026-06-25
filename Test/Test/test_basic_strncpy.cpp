#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strncpy_full_copy)
{
    char source[3];
    char destination[6];

    source[0] = 'h';
    source[1] = 'i';
    source[2] = '\0';
    destination[0] = 'x';
    destination[1] = 'x';
    destination[2] = 'x';
    destination[3] = 'x';
    destination[4] = 'x';
    destination[5] = '\0';
    FT_ASSERT_EQ(destination, ft_strncpy(destination, source, 5));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('i', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    FT_ASSERT_EQ('\0', destination[5]);
    return (1);
}

FT_TEST(test_basic_strncpy_truncate)
{
    char source[6];
    char destination[6];

    source[0] = 'h';
    source[1] = 'e';
    source[2] = 'l';
    source[3] = 'l';
    source[4] = 'o';
    source[5] = '\0';
    destination[0] = 'X';
    destination[1] = 'X';
    destination[2] = 'X';
    destination[3] = 'X';
    destination[4] = 'X';
    destination[5] = '\0';
    FT_ASSERT_EQ(destination, ft_strncpy(destination, source, 3));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('e', destination[1]);
    FT_ASSERT_EQ('l', destination[2]);
    FT_ASSERT_EQ('X', destination[3]);
    return (1);
}

FT_TEST(test_basic_strncpy_zero_length)
{
    char destination[4];

    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = 'c';
    destination[3] = '\0';
    FT_ASSERT_EQ(destination, ft_strncpy(destination, "xyz", 0));
    FT_ASSERT_EQ('a', destination[0]);
    return (1);
}

FT_TEST(test_basic_strncpy_zero_length_null_destination)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strncpy(ft_nullptr, "abc", 0));
    return (1);
}

FT_TEST(test_basic_strncpy_zero_length_null_source)
{
    char destination[2];

    destination[0] = 'v';
    destination[1] = '\0';
    FT_ASSERT_EQ(destination, ft_strncpy(destination, ft_nullptr, 0));
    FT_ASSERT_EQ('v', destination[0]);
    return (1);
}

FT_TEST(test_basic_strncpy_zero_length_both_null)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strncpy(ft_nullptr, ft_nullptr, 0));
    return (1);
}

FT_TEST(test_basic_strncpy_null)
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '\0';
    FT_ASSERT_EQ(ft_nullptr, ft_strncpy(ft_nullptr, buffer, 3));
    FT_ASSERT_EQ(ft_nullptr, ft_strncpy(buffer, ft_nullptr, 3));
    return (1);
}

FT_TEST(test_basic_strncpy_null_sets_errno)
{
    char buffer[4];

    buffer[0] = 'z';
    buffer[1] = '\0';
    FT_ASSERT_EQ(ft_nullptr, ft_strncpy(ft_nullptr, buffer, 2));
    FT_ASSERT_EQ(ft_nullptr, ft_strncpy(buffer, ft_nullptr, 2));
    return (1);
}

FT_TEST(test_basic_strncpy_zero_length_clears_errno)
{
    char destination[3];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = '\0';
    FT_ASSERT_EQ(destination, ft_strncpy(destination, "hi", 0));
    FT_ASSERT_EQ('x', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    return (1);
}

FT_TEST(test_basic_strncpy_pads_with_null_bytes)
{
    char source[3];
    char destination[5];

    source[0] = 'o';
    source[1] = 'k';
    source[2] = '\0';
    destination[0] = 'a';
    destination[1] = 'a';
    destination[2] = 'a';
    destination[3] = 'a';
    destination[4] = 'z';
    FT_ASSERT_EQ(destination, ft_strncpy(destination, source, 4));
    FT_ASSERT_EQ('o', destination[0]);
    FT_ASSERT_EQ('k', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    FT_ASSERT_EQ('z', destination[4]);
    return (1);
}

FT_TEST(test_basic_strncpy_partial_copy_preserves_tail)
{
    char destination[5];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = 'w';
    destination[4] = '\0';
    FT_ASSERT_EQ(destination, ft_strncpy(destination, "hi", 1));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    FT_ASSERT_EQ('z', destination[2]);
    FT_ASSERT_EQ('w', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    return (1);
}
