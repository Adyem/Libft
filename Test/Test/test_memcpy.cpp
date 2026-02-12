#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_memcpy_basic, "ft_memcpy copies bytes")
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

FT_TEST(test_memcpy_zero_length, "ft_memcpy zero-length is a no-op")
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

FT_TEST(test_memcpy_zero_length_null, "ft_memcpy null pointers are accepted for zero-length")
{
    FT_ASSERT_EQ(ft_nullptr, ft_memcpy(ft_nullptr, ft_nullptr, 0));
    return (1);
}

FT_TEST(test_memcpy_null_argument, "ft_memcpy returns null when a pointer is invalid")
{
    char source[2];

    source[0] = 'q';
    source[1] = '\0';
    FT_ASSERT_EQ(ft_nullptr, ft_memcpy(ft_nullptr, source, 1));
    FT_ASSERT_EQ(ft_nullptr, ft_memcpy(source, ft_nullptr, 1));
    return (1);
}

FT_TEST(test_memcpy_same_pointer, "ft_memcpy handles same source and destination")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    FT_ASSERT_EQ(buffer, ft_memcpy(buffer, buffer, 4));
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ('b', buffer[1]);
    FT_ASSERT_EQ('c', buffer[2]);
    FT_ASSERT_EQ('d', buffer[3]);
    return (1);
}

FT_TEST(test_memcpy_overlap_behavior, "ft_memcpy overlap keeps function usable afterward")
{
    char buffer[6];
    char source[4];
    char destination[4];

    buffer[0] = '1';
    buffer[1] = '2';
    buffer[2] = '3';
    buffer[3] = '4';
    buffer[4] = '5';
    buffer[5] = '\0';
    FT_ASSERT_EQ(buffer + 1, ft_memcpy(buffer + 1, buffer, 4));
    source[0] = 'd';
    source[1] = 'a';
    source[2] = 't';
    source[3] = 'a';
    destination[0] = 'x';
    destination[1] = 'x';
    destination[2] = 'x';
    destination[3] = '\0';
    FT_ASSERT_EQ(destination, ft_memcpy(destination, source, 3));
    FT_ASSERT_EQ('d', destination[0]);
    FT_ASSERT_EQ('a', destination[1]);
    FT_ASSERT_EQ('t', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    return (1);
}

FT_TEST(test_memcpy_partial_copy_preserves_tail, "ft_memcpy leaves bytes past copy length unchanged")
{
    char source[6];
    char destination[6];

    source[0] = 's';
    source[1] = 'o';
    source[2] = 'u';
    source[3] = 'r';
    source[4] = 'c';
    source[5] = 'e';
    destination[0] = 'd';
    destination[1] = 'e';
    destination[2] = 's';
    destination[3] = 't';
    destination[4] = '!';
    destination[5] = '\0';
    FT_ASSERT_EQ(destination, ft_memcpy(destination, source, 3));
    FT_ASSERT_EQ('s', destination[0]);
    FT_ASSERT_EQ('o', destination[1]);
    FT_ASSERT_EQ('u', destination[2]);
    FT_ASSERT_EQ('t', destination[3]);
    FT_ASSERT_EQ('!', destination[4]);
    FT_ASSERT_EQ('\0', destination[5]);
    return (1);
}
