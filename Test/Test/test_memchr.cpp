#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_memchr_found, "ft_memchr finds character")
{
    char buffer[4];
    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(buffer + 2, ft_memchr(buffer, 'c', 4));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_not_found, "ft_memchr missing character")
{
    char buffer[3];
    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(buffer, 'x', 3));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_null_char, "ft_memchr search for null")
{
    char buffer[4];
    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = '\0';
    buffer[3] = 'c';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(buffer + 2, ft_memchr(buffer, '\0', 4));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_zero_length, "ft_memchr zero length")
{
    char buffer[1];
    buffer[0] = 'a';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(buffer, 'a', 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_nullptr_zero, "ft_memchr nullptr zero length")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(ft_nullptr, 'a', 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_zero_length_clears_errno, "ft_memchr zero length clears errno")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(ft_nullptr, 'a', 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_null_sets_errno, "ft_memchr null pointer sets FT_ERR_INVALID_ARGUMENT")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(ft_nullptr, 'a', 1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_memchr_limit_stops_search, "ft_memchr respects length limit")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(buffer, 'c', 2));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_signed_byte, "ft_memchr matches signed byte values")
{
    char buffer[3];

    buffer[0] = 'a';
    buffer[1] = static_cast<char>(0xF2);
    buffer[2] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(buffer + 1, ft_memchr(buffer, 0xF2, 3));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_miss_clears_errno, "ft_memchr clears errno when byte missing")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(buffer, 'z', 4));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_recovers_after_null_failure, "ft_memchr clears errno after null pointer failure")
{
    char buffer[3];

    buffer[0] = 'q';
    buffer[1] = 'w';
    buffer[2] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(ft_nullptr, 'q', 2));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(buffer, ft_memchr(buffer, 'q', 2));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memchr_matches_last_byte, "ft_memchr can find values at the end of the range")
{
    char buffer[5];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    buffer[4] = 'e';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(buffer + 4, ft_memchr(buffer, 'e', 5));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
