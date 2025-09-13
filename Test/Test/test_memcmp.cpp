#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
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
    FT_ASSERT_EQ(0, ft_memcmp(a, b, 3));
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
    FT_ASSERT(ft_memcmp(a, b, 3) < 0);
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
    FT_ASSERT(ft_memcmp(a, b, 3) > 0);
    return (1);
}

FT_TEST(test_memcmp_zero_length, "ft_memcmp zero length")
{
    char a[1];
    char b[1];
    a[0] = 'x';
    b[0] = 'y';
    FT_ASSERT_EQ(0, ft_memcmp(a, b, 0));
    return (1);
}

FT_TEST(test_memcmp_high_bit, "ft_memcmp high-bit comparison")
{
    char a[1];
    char b[1];
    a[0] = (char)0x80;
    b[0] = 0;
    FT_ASSERT(ft_memcmp(a, b, 1) > 0);
    return (1);
}
