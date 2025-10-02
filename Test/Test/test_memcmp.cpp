#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_memcmp_equal, "ft_memcmp equal buffers")
{
    char a[3];
    char b[3];
    a[0] = 'a';
    a[1] = 'b';
    a[2] = 'c';
    b[0] = 'a';
    b[1] = 'b';
    b[2] = 'c';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, ft_memcmp(a, b, 3));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memcmp_less, "ft_memcmp less than")
{
    char a[3];
    char b[3];
    a[0] = 'a';
    a[1] = 'b';
    a[2] = 'c';
    b[0] = 'a';
    b[1] = 'b';
    b[2] = 'd';
    ft_errno = FT_EINVAL;
    FT_ASSERT(ft_memcmp(a, b, 3) < 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memcmp_greater, "ft_memcmp greater than")
{
    char a[3];
    char b[3];
    a[0] = 'a';
    a[1] = 'b';
    a[2] = 'e';
    b[0] = 'a';
    b[1] = 'b';
    b[2] = 'c';
    ft_errno = FT_EINVAL;
    FT_ASSERT(ft_memcmp(a, b, 3) > 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memcmp_zero_length, "ft_memcmp zero length")
{
    char a[1];
    char b[1];
    a[0] = 'x';
    b[0] = 'y';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, ft_memcmp(a, b, 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memcmp_zero_length_clears_errno, "ft_memcmp zero length clears errno")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, ft_memcmp(ft_nullptr, ft_nullptr, 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memcmp_high_bit, "ft_memcmp high-bit comparison")
{
    char a[1];
    char b[1];
    a[0] = (char)0x80;
    b[0] = 0;
    ft_errno = FT_EINVAL;
    FT_ASSERT(ft_memcmp(a, b, 1) > 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memcmp_limit_hides_late_difference, "ft_memcmp stops comparing after limit")
{
    char first[4];
    char second[4];

    first[0] = 'a';
    first[1] = 'b';
    first[2] = 'c';
    first[3] = '\0';
    second[0] = 'a';
    second[1] = 'b';
    second[2] = 'x';
    second[3] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, ft_memcmp(first, second, 2));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memcmp_null_pointer_sets_errno, "ft_memcmp null pointer sets FT_EINVAL")
{
    char buffer[2];

    buffer[0] = 'x';
    buffer[1] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_memcmp(buffer, ft_nullptr, 1));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_memcmp(ft_nullptr, buffer, 1));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}
