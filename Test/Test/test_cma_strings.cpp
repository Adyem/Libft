#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

static void cma_free_split_result(char **strings)
{
    ft_size_t index;

    if (!strings)
        return ;
    index = 0;
    while (strings[index])
    {
        cma_free(strings[index]);
        index++;
    }
    cma_free(strings);
    return ;
}

FT_TEST(test_cma_itoa_positive_number, "cma_itoa converts positive numbers")
{
    char *converted_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    converted_string = cma_itoa(12345);
    if (!converted_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(converted_string, "12345"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(converted_string);
    return (1);
}

FT_TEST(test_cma_itoa_handles_negative_and_zero,
        "cma_itoa preserves sign and handles zero")
{
    char *negative_string;
    char *zero_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    negative_string = cma_itoa(-2048);
    if (!negative_string)
        return (0);
    zero_string = cma_itoa(0);
    if (!zero_string)
    {
        cma_free(negative_string);
        return (0);
    }
    FT_ASSERT_EQ(0, ft_strcmp(negative_string, "-2048"));
    FT_ASSERT_EQ(0, ft_strcmp(zero_string, "0"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(negative_string);
    cma_free(zero_string);
    return (1);
}

FT_TEST(test_cma_itoa_handles_int_min, "cma_itoa duplicates INT_MIN literal")
{
    char *minimum_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    minimum_string = cma_itoa(FT_INT_MIN);
    if (!minimum_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(minimum_string, "-2147483648"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(minimum_string);
    return (1);
}

FT_TEST(test_cma_itoa_allocation_failure_sets_errno,
        "cma_itoa reports allocation failures")
{
    char *converted_string;

    cma_set_alloc_limit(2);
    ft_errno = ER_SUCCESS;
    converted_string = cma_itoa(9999);
    FT_ASSERT_EQ(ft_nullptr, converted_string);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_cma_itoa_base_hexadecimal,
        "cma_itoa_base converts numbers in hexadecimal")
{
    char *converted_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    converted_string = cma_itoa_base(305441741, 16);
    if (!converted_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(converted_string, "1234ABCD"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(converted_string);
    return (1);
}

FT_TEST(test_cma_itoa_base_negative_decimal,
        "cma_itoa_base prefixes negatives when base is decimal")
{
    char *converted_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    converted_string = cma_itoa_base(-256, 10);
    if (!converted_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(converted_string, "-256"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(converted_string);
    return (1);
}

FT_TEST(test_cma_itoa_base_rejects_invalid_base,
        "cma_itoa_base validates base range")
{
    char *converted_string;

    cma_set_alloc_limit(0);
    ft_errno = ER_SUCCESS;
    converted_string = cma_itoa_base(42, 1);
    FT_ASSERT_EQ(ft_nullptr, converted_string);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cma_strtrim_trims_prefix_and_suffix,
        "cma_strtrim removes leading and trailing characters")
{
    char *trimmed_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    trimmed_string = cma_strtrim("***hello***", "*");
    if (!trimmed_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(trimmed_string, "hello"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(trimmed_string);
    return (1);
}

FT_TEST(test_cma_strtrim_returns_empty_when_all_trimmed,
        "cma_strtrim returns an empty string when every character is trimmed")
{
    char *trimmed_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    trimmed_string = cma_strtrim("     ", " ");
    if (!trimmed_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(trimmed_string, ""));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(trimmed_string);
    return (1);
}

FT_TEST(test_cma_strtrim_rejects_null_input,
        "cma_strtrim returns nullptr when input or set is null")
{
    char *trimmed_string;

    cma_set_alloc_limit(0);
    ft_errno = ER_SUCCESS;
    trimmed_string = cma_strtrim(ft_nullptr, " ");
    FT_ASSERT_EQ(ft_nullptr, trimmed_string);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    trimmed_string = cma_strtrim("example", ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, trimmed_string);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cma_substr_within_bounds,
        "cma_substr extracts segments within the source bounds")
{
    char *substring;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    substring = cma_substr("abcdef", 2, 3);
    if (!substring)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(substring, "cde"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(substring);
    return (1);
}

FT_TEST(test_cma_substr_truncates_when_length_exceeds,
        "cma_substr truncates when requested length exceeds remaining characters")
{
    char *substring;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    substring = cma_substr("libft", 3, 10);
    if (!substring)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(substring, "ft"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(substring);
    return (1);
}

FT_TEST(test_cma_substr_handles_out_of_range_start,
        "cma_substr returns an empty string when start is outside the source")
{
    char *substring;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    substring = cma_substr("short", 42, 3);
    if (!substring)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(substring, ""));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(substring);
    return (1);
}

FT_TEST(test_cma_substr_rejects_null_source,
        "cma_substr returns nullptr when source is null")
{
    char *substring;

    cma_set_alloc_limit(0);
    ft_errno = ER_SUCCESS;
    substring = cma_substr(ft_nullptr, 0, 1);
    FT_ASSERT_EQ(ft_nullptr, substring);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cma_split_basic_tokens, "cma_split separates tokens and null-terminates the array")
{
    char **parts;
    ft_size_t index;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    parts = cma_split("alpha,beta,gamma", ',');
    if (!parts)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(parts[0], "alpha"));
    FT_ASSERT_EQ(0, ft_strcmp(parts[1], "beta"));
    FT_ASSERT_EQ(0, ft_strcmp(parts[2], "gamma"));
    FT_ASSERT_EQ(ft_nullptr, parts[3]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    index = 0;
    while (parts[index])
    {
        FT_ASSERT(parts[index] != ft_nullptr);
        index++;
    }
    cma_free_split_result(parts);
    return (1);
}

FT_TEST(test_cma_split_skips_repeated_delimiters, "cma_split ignores empty segments between delimiters")
{
    char **parts;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    parts = cma_split("::left::right:", ':');
    if (!parts)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(parts[0], "left"));
    FT_ASSERT_EQ(0, ft_strcmp(parts[1], "right"));
    FT_ASSERT_EQ(ft_nullptr, parts[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free_split_result(parts);
    return (1);
}

FT_TEST(test_cma_split_null_string_returns_empty_array, "cma_split returns an empty array when input is null")
{
    char **parts;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    parts = cma_split(ft_nullptr, ',');
    if (!parts)
        return (0);
    FT_ASSERT_EQ(ft_nullptr, parts[0]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(parts);
    return (1);
}

FT_TEST(test_cma_split_allocation_failure_sets_errno, "cma_split propagates allocation failures")
{
    char **parts;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    parts = cma_split("a,b", ',');
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, parts);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_cma_strdup_copies_string, "cma_strdup duplicates the input string")
{
    const char *source;
    char *duplicate;

    source = "duplicate target";
    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    duplicate = cma_strdup(source);
    if (!duplicate)
        return (0);
    FT_ASSERT(duplicate != source);
    FT_ASSERT_EQ(0, ft_strcmp(source, duplicate));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_strdup_null_input_preserves_errno, "cma_strdup returns nullptr without touching errno when input is null")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(ft_nullptr, cma_strdup(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cma_strdup_allocation_failure_sets_errno, "cma_strdup surfaces allocation errors")
{
    char *duplicate;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    duplicate = cma_strdup("needs space");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}
