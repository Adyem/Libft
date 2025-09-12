#include "../Template/string_view.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/test_runner.hpp"

FT_TEST(test_string_view_basic, "ft_string_view basic")
{
    ft_string_view<char> view("hello");
    FT_ASSERT_EQ(5u, view.size());
    FT_ASSERT_EQ('h', view.data()[0]);
    return (1);
}

FT_TEST(test_string_view_compare_substr, "ft_string_view compare and substr")
{
    ft_string_view<char> view("hello");
    ft_string_view<char> other_view("hello");
    FT_ASSERT_EQ(0, view.compare(other_view));
    ft_string_view<char> substring = view.substr(1, 3);
    ft_string_view<char> expected_view("ell");
    FT_ASSERT_EQ(0, substring.compare(expected_view));
    return (1);
}

FT_TEST(test_string_view_substr_oob, "ft_string_view substr out of bounds")
{
    ft_string_view<char> view("world");
    ft_string_view<char> substring = view.substr(10, 2);
    FT_ASSERT_EQ(FT_EINVAL, view.get_error());
    FT_ASSERT_EQ(FT_EINVAL, substring.get_error());
    FT_ASSERT_EQ(0u, substring.size());
    return (1);
}
