#include "../test_internal.hpp"
#include "../../Modules/Regex/regex.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_regex_count_matches_counts_non_overlapping_matches)
{
    ft_size_t match_count;

    match_count = 0U;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, regex_count_matches("[a-z]+",
            "one 22 two three", &match_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(3U), match_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, regex_count_matches("z", "abc",
            &match_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0U), match_count);
    return (1);
}

FT_TEST(test_regex_count_matches_reports_invalid_arguments_and_patterns)
{
    ft_size_t match_count;

    match_count = 99U;
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, regex_count_matches(ft_nullptr,
            "text", &match_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(99U), match_count);
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, regex_count_matches("a", ft_nullptr,
            &match_count));
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, regex_count_matches("a", "text",
            ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, regex_count_matches("[", "text",
            &match_count));
    return (1);
}

FT_TEST(test_regex_count_matches_handles_empty_text_and_zero_length_matches)
{
    ft_size_t match_count;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, regex_count_matches("a", "", &match_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0U), match_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, regex_count_matches("", "abc", &match_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(4U), match_count);
    return (1);
}
