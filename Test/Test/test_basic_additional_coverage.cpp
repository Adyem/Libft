#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_validate_int_sign_edge_cases,
    "ft_validate_int rejects sign-only values and accepts negative zero")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("-"));
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("+"));
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("-0"));
    return (1);
}

FT_TEST(test_span_dup_rejects_null_non_zero_length,
    "ft_span_dup rejects null buffers when length is non-zero")
{
    FT_ASSERT_EQ(ft_nullptr, ft_span_dup(ft_nullptr, 3));
    return (1);
}

FT_TEST(test_span_dup_accepts_null_zero_length,
    "ft_span_dup returns an empty string when null buffer is paired with zero length")
{
    char *duplicate;

    duplicate = ft_span_dup(ft_nullptr, 0);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ('\0', duplicate[0]);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_span_dup_reports_allocation_failure,
    "ft_span_dup returns null when allocation fails")
{
    char *duplicate;

    cma_set_alloc_limit(1);
    duplicate = ft_span_dup("abc", 3);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    return (1);
}

FT_TEST(test_strstr_matches_suffix,
    "ft_strstr finds needles located at the end of the haystack")
{
    const char *haystack;
    const char *needle;

    haystack = "prefix_suffix";
    needle = "suffix";
    FT_ASSERT_EQ(const_cast<char *>(haystack + 7), ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_strstr_rejects_longer_needle,
    "ft_strstr returns null when the needle is longer than the haystack")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strstr("abc", "abcdef"));
    return (1);
}

FT_TEST(test_strstr_empty_haystack_non_empty_needle,
    "ft_strstr returns null when haystack is empty and needle is non-empty")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strstr("", "a"));
    return (1);
}

FT_TEST(test_strnstr_respects_maximum_boundary,
    "ft_strnstr only matches when the full needle fits inside maximum length")
{
    const char *haystack;

    haystack = "abcde";
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(haystack, "cde", 4));
    FT_ASSERT_EQ(const_cast<char *>(haystack + 2), ft_strnstr(haystack, "cde", 5));
    return (1);
}

FT_TEST(test_strnlen_null_pointer_with_non_zero_limit,
    "ft_strnlen returns zero for null pointer regardless of maximum length")
{
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), ft_strnlen(ft_nullptr, 8));
    return (1);
}

FT_TEST(test_strnlen_zero_limit_with_non_empty_string,
    "ft_strnlen returns zero when maximum length is zero")
{
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), ft_strnlen("abcdef", 0));
    return (1);
}

FT_TEST(test_strnstr_zero_maximum_length,
    "ft_strnstr returns null when maximum length is zero and needle is non-empty")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("abc", "a", 0));
    return (1);
}

FT_TEST(test_strnstr_empty_needle_with_zero_maximum,
    "ft_strnstr returns haystack for empty needle even when maximum length is zero")
{
    const char *haystack;

    haystack = "abc";
    FT_ASSERT_EQ(const_cast<char *>(haystack), ft_strnstr(haystack, "", 0));
    return (1);
}

FT_TEST(test_strtok_only_delimiters_return_null,
    "ft_strtok returns null when input is delimiters only")
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

FT_TEST(test_memchr_character_is_unsigned_converted,
    "ft_memchr applies unsigned char conversion to search value")
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

FT_TEST(test_memchr_zero_length_ignores_pointer,
    "ft_memchr returns null for zero-length scans even with null pointer")
{
    FT_ASSERT_EQ(ft_nullptr, ft_memchr(ft_nullptr, 'x', 0));
    return (1);
}

FT_TEST(test_memchr_finds_zero_byte,
    "ft_memchr can locate a null byte inside binary data")
{
    unsigned char buffer[4];

    buffer[0] = 'a';
    buffer[1] = '\0';
    buffer[2] = 'b';
    buffer[3] = '\0';
    FT_ASSERT_EQ(buffer + 1, ft_memchr(buffer, '\0', 4));
    return (1);
}

FT_TEST(test_strchr_wraps_search_value_to_char,
    "ft_strchr matches search values after char conversion")
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

FT_TEST(test_strrchr_wraps_search_value_to_char,
    "ft_strrchr matches search values after char conversion")
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
FT_TEST(test_locale_compare_equal_values,
    "ft_locale_compare returns zero for equivalent strings")
{
    FT_ASSERT_EQ(0, ft_locale_compare("same", "same", "C"));
    return (1);
}

FT_TEST(test_locale_compare_rejects_right_null,
    "ft_locale_compare rejects null right-hand input")
{
    FT_ASSERT_EQ(-1, ft_locale_compare("value", ft_nullptr, "C"));
    return (1);
}

FT_TEST(test_locale_compare_reverse_ordering,
    "ft_locale_compare returns positive value when left is greater than right")
{
    FT_ASSERT(ft_locale_compare("zeta", "alpha", "C") > 0);
    return (1);
}
#endif
