#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strncpy_full_copy, "ft_strncpy full copy with padding")
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

FT_TEST(test_strncpy_truncate, "ft_strncpy truncates without null")
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

FT_TEST(test_strncpy_zero_length, "ft_strncpy zero length")
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

FT_TEST(test_strncpy_null, "ft_strncpy with nullptr")
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
