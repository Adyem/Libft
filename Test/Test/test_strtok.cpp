#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strtok_basic, "ft_strtok basic")
{
    char buffer[14] = "one two three";
    char *token;

    token = ft_strtok(buffer, " ");
    FT_ASSERT_EQ(0, ft_strcmp("one", token));
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT_EQ(0, ft_strcmp("two", token));
    token = ft_strtok(ft_nullptr, " ");
    FT_ASSERT_EQ(0, ft_strcmp("three", token));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, " "));
    return (1);
}

FT_TEST(test_strtok_edge, "ft_strtok edge")
{
    char buffer[4] = ",,,";
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(buffer, ","));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ","));

    char second[4] = "abc";
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(second, ft_nullptr));
    return (1);
}
