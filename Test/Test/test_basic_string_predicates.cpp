#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_strcasecmp_compares_ascii_case_insensitively)
{
    FT_ASSERT_EQ(0, ft_strcasecmp("Alpha", "alpha"));
    FT_ASSERT(ft_strcasecmp("Alpha", "beta") < 0);
    FT_ASSERT(ft_strcasecmp("beta", "Alpha") > 0);
    return (1);
}

FT_TEST(test_strncasecmp_limits_the_comparison_length)
{
    FT_ASSERT_EQ(0, ft_strncasecmp("Alphabet", "alpha", 5));
    FT_ASSERT(ft_strncasecmp("Alphabet", "alphi", 5) < 0);
    FT_ASSERT_EQ(0, ft_strncasecmp("short", "shorter", 5));
    return (1);
}

FT_TEST(test_str_starts_with_matches_prefixes)
{
    FT_ASSERT_EQ(FT_TRUE, ft_str_starts_with("library", "lib"));
    FT_ASSERT_EQ(FT_FALSE, ft_str_starts_with("library", "rib"));
    FT_ASSERT_EQ(FT_TRUE, ft_str_starts_with("library", ""));
    return (1);
}

FT_TEST(test_str_ends_with_matches_suffixes)
{
    FT_ASSERT_EQ(FT_TRUE, ft_str_ends_with("library", "ary"));
    FT_ASSERT_EQ(FT_FALSE, ft_str_ends_with("library", "lib"));
    FT_ASSERT_EQ(FT_TRUE, ft_str_ends_with("library", ""));
    return (1);
}

FT_TEST(test_str_contains_finds_substrings)
{
    FT_ASSERT_EQ(FT_TRUE, ft_str_contains("full libft", "libft"));
    FT_ASSERT_EQ(FT_FALSE, ft_str_contains("full libft", "basic"));
    FT_ASSERT_EQ(FT_TRUE, ft_str_contains("full libft", ""));
    return (1);
}

FT_TEST(test_string_predicates_handle_null_pointers)
{
    FT_ASSERT_EQ(-1, ft_strcasecmp(ft_nullptr, "alpha"));
    FT_ASSERT_EQ(-1, ft_strncasecmp("alpha", ft_nullptr, 3));
    FT_ASSERT_EQ(FT_FALSE, ft_str_starts_with(ft_nullptr, "a"));
    FT_ASSERT_EQ(FT_FALSE, ft_str_ends_with("a", ft_nullptr));
    FT_ASSERT_EQ(FT_FALSE, ft_str_contains(ft_nullptr, "a"));
    return (1);
}
