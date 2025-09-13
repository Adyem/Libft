#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strrchr_last, "ft_strrchr last occurrence")
{
    const char *string = "banana";

    FT_ASSERT_EQ(string + 5, ft_strrchr(string, 'a'));
    return (1);
}

FT_TEST(test_strrchr_not_found, "ft_strrchr not found")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strrchr("hello", 'x'));
    return (1);
}

FT_TEST(test_strrchr_null, "ft_strrchr nullptr")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strrchr(ft_nullptr, 'a'));
    return (1);
}

FT_TEST(test_strrchr_terminator, "ft_strrchr terminator")
{
    const char *string = "world";

    FT_ASSERT_EQ(string + 5, ft_strrchr(string, '\0'));
    return (1);
}
