#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strlcat_basic, "ft_strlcat basic")
{
    char destination[10];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    FT_ASSERT_EQ(6u, ft_strlcat(destination, "1234", 10));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hi1234"));
    return (1);
}

FT_TEST(test_strlcat_truncate, "ft_strlcat truncate")
{
    char destination[6];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    FT_ASSERT_EQ(7u, ft_strlcat(destination, "world", 6));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hiwor"));
    return (1);
}

FT_TEST(test_strlcat_zero_size, "ft_strlcat zero size")
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

FT_TEST(test_strlcat_insufficient_dest, "ft_strlcat size less than dest length")
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

