#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strlcpy_basic)
{
    char destination[6];

    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 6));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    return (1);
}

FT_TEST(test_basic_strlcpy_truncate)
{
    char destination[3];

    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 3));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('e', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_basic_strlcpy_zero)
{
    char destination[4];

    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = 'c';
    destination[3] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 0));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('c', destination[2]);
    return (1);
}

FT_TEST(test_basic_strlcpy_zero_size_null_destination)
{
    FT_ASSERT_EQ(5u, ft_strlcpy(ft_nullptr, "hello", 0));
    return (1);
}

FT_TEST(test_basic_strlcpy_one_byte_buffer)
{
    char destination[4];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 1));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    return (1);
}

FT_TEST(test_basic_strlcpy_null_arguments)
{
    char destination[4];

    destination[0] = 'n';
    destination[1] = '\0';
    FT_ASSERT_EQ(0u, ft_strlcpy(ft_nullptr, "abc", 4));
    FT_ASSERT_EQ(0u, ft_strlcpy(destination, ft_nullptr, 4));
    return (1);
}

FT_TEST(test_basic_strlcpy_empty_source)
{
    char destination[5];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = 'w';
    destination[4] = '\0';
    FT_ASSERT_EQ(0u, ft_strlcpy(destination, "", sizeof(destination)));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    return (1);
}

FT_TEST(test_basic_strlcpy_preserves_tail)
{
    char destination[8];

    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = 'c';
    destination[3] = 'd';
    destination[4] = 'e';
    destination[5] = 'x';
    destination[6] = 'y';
    destination[7] = 'z';
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    FT_ASSERT_EQ('y', destination[6]);
    FT_ASSERT_EQ('z', destination[7]);
    return (1);
}

FT_TEST(test_basic_strlcpy_resets_errno_after_null_source)
{
    char destination[6];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = 'w';
    destination[4] = 'v';
    destination[5] = '\0';
    FT_ASSERT_EQ(0u, ft_strlcpy(destination, ft_nullptr, sizeof(destination)));
    FT_ASSERT_EQ(4u, ft_strlcpy(destination, "test", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "test"));
    return (1);
}

FT_TEST(test_basic_strlcpy_resets_errno)
{
    char destination[6];

    destination[0] = '1';
    destination[1] = '2';
    destination[2] = '3';
    destination[3] = '4';
    destination[4] = '5';
    destination[5] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    return (1);
}

FT_TEST(test_basic_strlcpy_self_copy)
{
    char buffer[6];

    buffer[0] = 'h';
    buffer[1] = 'e';
    buffer[2] = 'l';
    buffer[3] = 'l';
    buffer[4] = 'o';
    buffer[5] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcpy(buffer, buffer, sizeof(buffer)));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "hello"));
    return (1);
}

FT_TEST(test_basic_strlcpy_two_byte_buffer)
{
    char destination[4];

    destination[0] = 'p';
    destination[1] = 'q';
    destination[2] = 'r';
    destination[3] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 2));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('r', destination[2]);
    return (1);
}

FT_TEST(test_basic_strlcpy_zero_size_resets_errno)
{
    char destination[3];

    destination[0] = 'm';
    destination[1] = 'n';
    destination[2] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 0));
    FT_ASSERT_EQ('m', destination[0]);
    return (1);
}

FT_TEST(test_basic_strlcpy_buffer_matches_source_length)
{
    char destination[5];

    destination[0] = 'q';
    destination[1] = 'q';
    destination[2] = 'q';
    destination[3] = 'q';
    destination[4] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", sizeof(destination)));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('e', destination[1]);
    FT_ASSERT_EQ('l', destination[2]);
    FT_ASSERT_EQ('l', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    return (1);
}

FT_TEST(test_basic_strlcpy_embedded_null_source)
{
    char source[4];
    char destination[3];

    source[0] = 'a';
    source[1] = '\0';
    source[2] = 'b';
    source[3] = '\0';
    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    FT_ASSERT_EQ(1u, ft_strlcpy(destination, source, sizeof(destination)));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('z', destination[2]);
    return (1);
}

FT_TEST(test_basic_strlcpy_long_source_counts_full_length)
{
    const char *source;
    char destination[5];

    source = "abcdefghijklmnopqrstuvwxyz";
    destination[0] = 'r';
    destination[1] = 's';
    destination[2] = 't';
    destination[3] = 'u';
    destination[4] = 'v';
    FT_ASSERT_EQ(26u, ft_strlcpy(destination, source, sizeof(destination)));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('b', destination[1]);
    FT_ASSERT_EQ('c', destination[2]);
    FT_ASSERT_EQ('d', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    return (1);
}

FT_TEST(test_basic_strlcpy_recovers_after_null_destination_failure)
{
    char destination[6];

    destination[0] = '0';
    destination[1] = '0';
    destination[2] = '0';
    destination[3] = '0';
    destination[4] = '0';
    destination[5] = '\0';
    FT_ASSERT_EQ(0u, ft_strlcpy(ft_nullptr, "hello", sizeof(destination)));
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    return (1);
}
