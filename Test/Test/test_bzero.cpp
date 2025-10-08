#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_bzero_basic, "ft_bzero basic")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_bzero(buffer, 4);
    FT_ASSERT_EQ(0, buffer[0]);
    FT_ASSERT_EQ(0, buffer[1]);
    FT_ASSERT_EQ(0, buffer[2]);
    FT_ASSERT_EQ(0, buffer[3]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_bzero_partial, "ft_bzero partial")
{
    char buffer[5];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    buffer[4] = 'e';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_bzero(buffer + 1, 3);
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ(0, buffer[1]);
    FT_ASSERT_EQ(0, buffer[2]);
    FT_ASSERT_EQ(0, buffer[3]);
    FT_ASSERT_EQ('e', buffer[4]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_bzero_zero_length, "ft_bzero zero length")
{
    char buffer[3];

    buffer[0] = 'x';
    buffer[1] = 'y';
    buffer[2] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_bzero(buffer, 0);
    FT_ASSERT_EQ('x', buffer[0]);
    FT_ASSERT_EQ('y', buffer[1]);
    FT_ASSERT_EQ('\0', buffer[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_bzero_null_zero, "ft_bzero nullptr zero")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_bzero(ft_nullptr, 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_bzero_null_sets_errno, "ft_bzero null pointer with length sets errno")
{
    ft_errno = ER_SUCCESS;
    ft_bzero(ft_nullptr, 4);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_bzero_recovers_errno_after_error, "ft_bzero clears errno after recovering from error")
{
    char buffer[3];

    buffer[0] = 'x';
    buffer[1] = 'y';
    buffer[2] = 'z';
    ft_errno = ER_SUCCESS;
    ft_bzero(ft_nullptr, 2);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_bzero(buffer, 2);
    FT_ASSERT_EQ(0, buffer[0]);
    FT_ASSERT_EQ(0, buffer[1]);
    FT_ASSERT_EQ('z', buffer[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

