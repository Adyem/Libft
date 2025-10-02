#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strlcat_basic, "ft_strlcat basic")
{
    char destination[10];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(6u, ft_strlcat(destination, "1234", 10));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hi1234"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcat_truncate, "ft_strlcat truncate")
{
    char destination[6];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(7u, ft_strlcat(destination, "world", 6));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hiwor"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcat_zero_size, "ft_strlcat zero size")
{
    char destination[3];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(5u, ft_strlcat(destination, "hello", 0));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('i', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcat_insufficient_dest, "ft_strlcat size less than dest length")
{
    char destination[6];

    destination[0] = 'h';
    destination[1] = 'e';
    destination[2] = 'l';
    destination[3] = 'l';
    destination[4] = 'o';
    destination[5] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(8u, ft_strlcat(destination, "world", 3));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlcat_null_arguments_errno, "ft_strlcat null arguments set errno")
{
    char destination_buffer[4];

    destination_buffer[0] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0u, ft_strlcat(ft_nullptr, "abc", 3));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0u, ft_strlcat(destination_buffer, ft_nullptr, 3));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_strlcat_errno_resets_on_success, "ft_strlcat resets errno on success")
{
    char destination_buffer[10];

    destination_buffer[0] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(5u, ft_strlcat(destination_buffer, "hello", 10));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, ft_strcmp(destination_buffer, "hello"));
    return (1);
}

FT_TEST(test_strlcat_truncated_destination, "ft_strlcat handles unterminated destination within limit")
{
    char destination[7];

    destination[0] = 'A';
    destination[1] = 'B';
    destination[2] = 'C';
    destination[3] = 'D';
    destination[4] = 'Y';
    destination[5] = 'Z';
    destination[6] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(6u, ft_strlcat(destination, "pq", 4));
    FT_ASSERT_EQ('A', destination[0]);
    FT_ASSERT_EQ('D', destination[3]);
    FT_ASSERT_EQ('Y', destination[4]);
    FT_ASSERT_EQ('Z', destination[5]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

