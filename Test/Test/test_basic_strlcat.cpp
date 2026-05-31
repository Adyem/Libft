#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strlcat_basic)
{
    char destination[10];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    FT_ASSERT_EQ(6u, ft_strlcat(destination, "1234", 10));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hi1234"));
    return (1);
}

FT_TEST(test_basic_strlcat_truncate)
{
    char destination[6];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    FT_ASSERT_EQ(7u, ft_strlcat(destination, "world", 6));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hiwor"));
    return (1);
}

FT_TEST(test_basic_strlcat_zero_size)
{
    char destination[3];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcat(destination, "hello", 0));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('i', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_basic_strlcat_zero_size_null_destination)
{
    FT_ASSERT_EQ(5u, ft_strlcat(ft_nullptr, "hello", 0));
    return (1);
}

FT_TEST(test_basic_strlcat_insufficient_dest)
{
    char destination[6];

    destination[0] = 'h';
    destination[1] = 'e';
    destination[2] = 'l';
    destination[3] = 'l';
    destination[4] = 'o';
    destination[5] = '\0';
    FT_ASSERT_EQ(8u, ft_strlcat(destination, "world", 3));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    return (1);
}

FT_TEST(test_basic_strlcat_null_arguments_errno)
{
    char destination_buffer[4];

    destination_buffer[0] = '\0';
    FT_ASSERT_EQ(0u, ft_strlcat(ft_nullptr, "abc", 3));
    FT_ASSERT_EQ(0u, ft_strlcat(destination_buffer, ft_nullptr, 3));
    return (1);
}

FT_TEST(test_basic_strlcat_errno_resets_on_success)
{
    char destination_buffer[10];

    destination_buffer[0] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcat(destination_buffer, "hello", 10));
    FT_ASSERT_EQ(0, ft_strcmp(destination_buffer, "hello"));
    return (1);
}

FT_TEST(test_basic_strlcat_truncated_destination)
{
    char destination[7];

    destination[0] = 'A';
    destination[1] = 'B';
    destination[2] = 'C';
    destination[3] = 'D';
    destination[4] = 'Y';
    destination[5] = 'Z';
    destination[6] = '\0';
    FT_ASSERT_EQ(6u, ft_strlcat(destination, "pq", 4));
    FT_ASSERT_EQ('A', destination[0]);
    FT_ASSERT_EQ('D', destination[3]);
    FT_ASSERT_EQ('Y', destination[4]);
    FT_ASSERT_EQ('Z', destination[5]);
    return (1);
}

FT_TEST(test_basic_strlcat_unterminated_buffer_exact_size)
{
    char destination[5];
    const char *source;
    size_t result;

    destination[0] = '1';
    destination[1] = '2';
    destination[2] = '3';
    destination[3] = '4';
    destination[4] = '5';
    source = "abc";
    result = ft_strlcat(destination, source, 5);
    FT_ASSERT_EQ(8u, result);
    FT_ASSERT_EQ('1', destination[0]);
    FT_ASSERT_EQ('2', destination[1]);
    FT_ASSERT_EQ('3', destination[2]);
    FT_ASSERT_EQ('4', destination[3]);
    FT_ASSERT_EQ('5', destination[4]);
    return (1);
}

FT_TEST(test_basic_strlcat_recovers_after_null_source)
{
    char destination[8];

    destination[0] = 'A';
    destination[1] = '\0';
    FT_ASSERT_EQ(0u, ft_strlcat(destination, ft_nullptr, 4));
    destination[0] = 'B';
    destination[1] = '\0';
    FT_ASSERT_EQ(5u, ft_strlcat(destination, "test", 8));
    FT_ASSERT_EQ(0, ft_strcmp("Btest", destination));
    return (1);
}

FT_TEST(test_basic_strlcat_empty_source_returns_existing_length)
{
    char destination[6];

    destination[0] = 'h';
    destination[1] = 'e';
    destination[2] = 'y';
    destination[3] = '\0';
    destination[4] = 'x';
    destination[5] = 'x';
    FT_ASSERT_EQ(3u, ft_strlcat(destination, "", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hey"));
    return (1);
}

FT_TEST(test_basic_strlcat_size_one_with_empty_destination)
{
    char destination[1];

    destination[0] = '\0';
    FT_ASSERT_EQ(3u, ft_strlcat(destination, "abc", sizeof(destination)));
    FT_ASSERT_EQ('\0', destination[0]);
    return (1);
}

FT_TEST(test_basic_strlcat_size_one_with_non_empty_buffer)
{
    char destination[1];

    destination[0] = 'x';
    FT_ASSERT_EQ(4u, ft_strlcat(destination, "abc", sizeof(destination)));
    FT_ASSERT_EQ('x', destination[0]);
    return (1);
}
