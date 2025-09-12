#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/test_runner.hpp"

FT_TEST(test_strcmp_equal, "ft_strcmp equal strings")
{
    FT_ASSERT_EQ(0, ft_strcmp("abc", "abc"));
    return (1);
}

FT_TEST(test_strcmp_null, "ft_strcmp with nullptr")
{
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, "abc"));
    return (1);
}
