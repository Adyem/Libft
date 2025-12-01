#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strlcpy_basic, "ft_strlcpy basic")
{
    char destination[6];

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 6));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_truncate, "ft_strlcpy truncate")
{
    char destination[3];

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 3));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('e', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_zero, "ft_strlcpy zero size")
{
    char destination[4];

    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = 'c';
    destination[3] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 0));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('c', destination[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_zero_size_null_destination, "ft_strlcpy zero size allows null destination")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(ft_nullptr, "hello", 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_one_byte_buffer, "ft_strlcpy buffer size one")
{
    char destination[4];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 1));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_null_arguments, "ft_strlcpy null arguments set FT_ERR_INVALID_ARGUMENT")
{
    char destination[4];

    destination[0] = 'n';
    destination[1] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0u, ft_strlcpy(ft_nullptr, "abc", 4));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0u, ft_strlcpy(destination, ft_nullptr, 4));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_empty_source, "ft_strlcpy handles empty source")
{
    char destination[5];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = 'w';
    destination[4] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0u, ft_strlcpy(destination, "", sizeof(destination)));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_preserves_tail, "ft_strlcpy preserves bytes beyond terminator")
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
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    FT_ASSERT_EQ('y', destination[6]);
    FT_ASSERT_EQ('z', destination[7]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_resets_errno_after_null_source, "ft_strlcpy clears errno after null source failure")
{
    char destination[6];

    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = 'w';
    destination[4] = 'v';
    destination[5] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0u, ft_strlcpy(destination, ft_nullptr, sizeof(destination)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(4u, ft_strlcpy(destination, "test", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "test"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_resets_errno, "ft_strlcpy resets errno on success")
{
    char destination[6];

    destination[0] = '1';
    destination[1] = '2';
    destination[2] = '3';
    destination[3] = '4';
    destination[4] = '5';
    destination[5] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_self_copy, "ft_strlcpy supports identical buffers")
{
    char buffer[6];

    buffer[0] = 'h';
    buffer[1] = 'e';
    buffer[2] = 'l';
    buffer[3] = 'l';
    buffer[4] = 'o';
    buffer[5] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(buffer, buffer, sizeof(buffer)));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "hello"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_two_byte_buffer, "ft_strlcpy truncates with size two")
{
    char destination[4];

    destination[0] = 'p';
    destination[1] = 'q';
    destination[2] = 'r';
    destination[3] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 2));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('r', destination[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_zero_size_resets_errno, "ft_strlcpy zero size resets errno")
{
    char destination[3];

    destination[0] = 'm';
    destination[1] = 'n';
    destination[2] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 0));
    FT_ASSERT_EQ('m', destination[0]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_buffer_matches_source_length, "ft_strlcpy buffer equals source length")
{
    char destination[5];

    destination[0] = 'q';
    destination[1] = 'q';
    destination[2] = 'q';
    destination[3] = 'q';
    destination[4] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", sizeof(destination)));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('e', destination[1]);
    FT_ASSERT_EQ('l', destination[2]);
    FT_ASSERT_EQ('l', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_embedded_null_source, "ft_strlcpy stops at embedded null")
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
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1u, ft_strlcpy(destination, source, sizeof(destination)));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('z', destination[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_long_source_counts_full_length, "ft_strlcpy counts full source length when truncated")
{
    const char *source;
    char destination[5];

    source = "abcdefghijklmnopqrstuvwxyz";
    destination[0] = 'r';
    destination[1] = 's';
    destination[2] = 't';
    destination[3] = 'u';
    destination[4] = 'v';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(26u, ft_strlcpy(destination, source, sizeof(destination)));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('b', destination[1]);
    FT_ASSERT_EQ('c', destination[2]);
    FT_ASSERT_EQ('d', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcpy_recovers_after_null_destination_failure, "ft_strlcpy resets errno after failure")
{
    char destination[6];

    destination[0] = '0';
    destination[1] = '0';
    destination[2] = '0';
    destination[3] = '0';
    destination[4] = '0';
    destination[5] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0u, ft_strlcpy(ft_nullptr, "hello", sizeof(destination)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", sizeof(destination)));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
