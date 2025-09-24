#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_memcpy_basic, "ft_memcpy basic")
{
    char source[6];
    char destination[6];

    source[0] = 'h';
    source[1] = 'e';
    source[2] = 'l';
    source[3] = 'l';
    source[4] = 'o';
    source[5] = '\0';
    FT_ASSERT_EQ(destination, ft_memcpy(destination, source, 6));
    FT_ASSERT_EQ(0, ft_strcmp(destination, source));
    return (1);
}

FT_TEST(test_memcpy_zero_length, "ft_memcpy zero length")
{
    char source[4];
    char destination[4];

    source[0] = 'x';
    source[1] = 'y';
    source[2] = 'z';
    source[3] = '\0';
    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = 'c';
    destination[3] = '\0';
    FT_ASSERT_EQ(destination, ft_memcpy(destination, source, 0));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "abc"));
    return (1);
}

FT_TEST(test_memcpy_zero_length_nullptr, "ft_memcpy zero length with nullptr")
{
    FT_ASSERT_EQ(ft_nullptr, ft_memcpy(ft_nullptr, ft_nullptr, 0));
    return (1);
}

FT_TEST(test_memcpy_null, "ft_memcpy with nullptr")
{
    char source[1];
    source[0] = 'a';
    FT_ASSERT_EQ(ft_nullptr, ft_memcpy(ft_nullptr, source, 1));
    FT_ASSERT_EQ(ft_nullptr, ft_memcpy(source, ft_nullptr, 1));
    return (1);
}

FT_TEST(test_memcpy_same_pointer, "ft_memcpy same pointer")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    FT_ASSERT_EQ(buffer, ft_memcpy(buffer, buffer, 4));
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ('d', buffer[3]);
    return (1);
}
