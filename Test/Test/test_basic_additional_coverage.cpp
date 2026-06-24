#include "../test_internal.hpp"
#include "../../Modules/Basic/config.hpp"
#include "../../Modules/Advanced/advanced.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_validate_int_sign_edge_cases)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_validate_int("-"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_validate_int("+"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_validate_int("-0"));
    return (1);
}

FT_TEST(test_basic_span_dup_rejects_null_non_zero_length)
{
    FT_ASSERT_EQ(ft_nullptr, ft_span_dup(ft_nullptr, 3));
    return (1);
}

FT_TEST(test_basic_span_dup_accepts_null_zero_length)
{
    char *duplicate;

    duplicate = ft_span_dup(ft_nullptr, 0);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ('\0', duplicate[0]);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_basic_span_dup_reports_allocation_failure)
{
    char *duplicate;

    cma_set_alloc_limit(1);
    duplicate = ft_span_dup("abc", 3);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    return (1);
}

FT_TEST(test_basic_strstr_matches_suffix)
{
    const char *haystack;
    const char *needle;

    haystack = "prefix_suffix";
    needle = "suffix";
    FT_ASSERT_EQ(const_cast<char *>(haystack + 7), ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_basic_strstr_rejects_longer_needle)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strstr("abc", "abcdef"));
    return (1);
}

FT_TEST(test_basic_strstr_empty_haystack_non_empty_needle)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strstr("", "a"));
    return (1);
}

FT_TEST(test_basic_strnstr_respects_maximum_boundary)
{
    const char *haystack;

    haystack = "abcde";
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(haystack, "cde", 4));
    FT_ASSERT_EQ(const_cast<char *>(haystack + 2), ft_strnstr(haystack, "cde", 5));
    return (1);
}

FT_TEST(test_basic_strnlen_null_pointer_with_non_zero_limit)
{
    FT_ASSERT_EQ(0, ft_strnlen(ft_nullptr, 8));
    return (1);
}

FT_TEST(test_basic_strnlen_zero_limit_with_non_empty_string)
{
    FT_ASSERT_EQ(0, ft_strnlen("abcdef", 0));
    return (1);
}

FT_TEST(test_basic_strnstr_zero_maximum_length)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("abc", "a", 0));
    return (1);
}

FT_TEST(test_basic_strnstr_empty_needle_with_zero_maximum)
{
    const char *haystack;

    haystack = "abc";
    FT_ASSERT_EQ(const_cast<char *>(haystack), ft_strnstr(haystack, "", 0));
    return (1);
}

FT_TEST(test_basic_strtok_only_delimiters_return_null)
{
    char buffer[5];

    buffer[0] = ',';
    buffer[1] = ',';
    buffer[2] = ',';
    buffer[3] = ',';
    buffer[4] = '\0';
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(buffer, ","));
    FT_ASSERT_EQ(ft_nullptr, ft_strtok(ft_nullptr, ","));
    return (1);
}

FT_TEST(test_basic_memchr_character_is_unsigned_converted)
{
    unsigned char buffer[4];

    buffer[0] = 0x41;
    buffer[1] = 0xFF;
    buffer[2] = 0x10;
    buffer[3] = 0x20;
    FT_ASSERT_EQ(buffer + 1, ft_memchr(buffer, -1, 4));
    FT_ASSERT_EQ(buffer + 1, ft_memchr(buffer, 0x1FF, 4));
    return (1);
}

FT_TEST(test_basic_memchr_zero_length_ignores_pointer)
{
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(ft_nullptr, 'x', 0));
    return (1);
}

FT_TEST(test_basic_memchr_finds_zero_byte)
{
    unsigned char buffer[4];

    buffer[0] = 'a';
    buffer[1] = '\0';
    buffer[2] = 'b';
    buffer[3] = '\0';
    FT_ASSERT_EQ(buffer + 1, ft_memchr(buffer, '\0', 4));
    return (1);
}

FT_TEST(test_basic_strchr_wraps_search_value_to_char)
{
    unsigned char string[4];
    const char *string_as_char;

    string[0] = 0x61;
    string[1] = 0xFF;
    string[2] = 0x62;
    string[3] = '\0';
    string_as_char = reinterpret_cast<const char *>(string);
    FT_ASSERT_EQ(const_cast<char *>(string_as_char + 1), ft_strchr(string_as_char, 0x1FF));
    return (1);
}

FT_TEST(test_basic_strrchr_wraps_search_value_to_char)
{
    unsigned char string[5];
    const char *string_as_char;

    string[0] = 0x10;
    string[1] = 0xFF;
    string[2] = 0x20;
    string[3] = 0xFF;
    string[4] = '\0';
    string_as_char = reinterpret_cast<const char *>(string);
    FT_ASSERT_EQ(const_cast<char *>(string_as_char + 3), ft_strrchr(string_as_char, 0x1FF));
    return (1);
}

#if LIBFT_HAS_LOCALE_HELPERS
FT_TEST(test_basic_locale_compare_equal_values)
{
    FT_ASSERT_EQ(0, ft_locale_compare("same", "same", "C"));
    return (1);
}

FT_TEST(test_basic_locale_compare_rejects_right_null)
{
    FT_ASSERT_EQ(-1, ft_locale_compare("value", ft_nullptr, "C"));
    return (1);
}

FT_TEST(test_basic_locale_compare_reverse_ordering)
{
    FT_ASSERT(ft_locale_compare("zeta", "alpha", "C") > 0);
    return (1);
}
#endif
