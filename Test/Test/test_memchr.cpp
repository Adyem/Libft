#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_memchr_found, "ft_memchr finds character")
{
    char buffer[4];
    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    FT_ASSERT_EQ(buffer + 2, ft_memchr(buffer, 'c', 4));
    return (1);
}

FT_TEST(test_memchr_not_found, "ft_memchr missing character")
{
    char buffer[3];
    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(buffer, 'x', 3));
    return (1);
}

FT_TEST(test_memchr_null_char, "ft_memchr search for null")
{
    char buffer[4];
    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = '\0';
    buffer[3] = 'c';
    FT_ASSERT_EQ(buffer + 2, ft_memchr(buffer, '\0', 4));
    return (1);
}

FT_TEST(test_memchr_zero_length, "ft_memchr zero length")
{
    char buffer[1];
    buffer[0] = 'a';
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(buffer, 'a', 0));
    return (1);
}

FT_TEST(test_memchr_nullptr_zero, "ft_memchr nullptr zero length")
{
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(ft_nullptr, 'a', 0));
    return (1);
}
