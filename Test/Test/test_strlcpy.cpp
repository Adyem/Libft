#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strlcpy_basic, "ft_strlcpy basic")
{
    char destination[6];

    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 6));
    FT_ASSERT_EQ(0, ft_strcmp(destination, "hello"));
    return (1);
}

FT_TEST(test_strlcpy_truncate, "ft_strlcpy truncate")
{
    char destination[3];

    FT_ASSERT_EQ(5u, ft_strlcpy(destination, "hello", 3));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('e', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_strlcpy_zero, "ft_strlcpy zero size")
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
