#include "../test_internal.hpp"
#include "../../Advanced/advanced.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static void free_split_result(char **split_result)
{
    ft_size_t index;

    if (split_result == ft_nullptr)
        return ;
    index = 0;
    while (split_result[index] != ft_nullptr)
    {
        cma_free(split_result[index]);
        index += 1;
    }
    cma_free(split_result);
    return ;
}

static ft_size_t advanced_current_allocated_bytes(void)
{
    ft_size_t allocation_count;
    ft_size_t free_count;
    ft_size_t current_bytes;
    ft_size_t peak_bytes;
    int32_t status;

    allocation_count = 0;
    free_count = 0;
    current_bytes = 0;
    peak_bytes = 0;
    status = cma_get_extended_stats(&allocation_count, &free_count,
            &current_bytes, &peak_bytes);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, status);
    return (current_bytes);
}

FT_TEST(test_adv_strjoin_handles_null_and_values,
    "adv_strjoin concatenates inputs and treats null as empty")
{
    char *joined_value;
    char *null_prefixed_value;

    joined_value = adv_strjoin("foo", "bar");
    null_prefixed_value = adv_strjoin(ft_nullptr, "bar");
    FT_ASSERT(joined_value != ft_nullptr);
    FT_ASSERT(null_prefixed_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("foobar", joined_value));
    FT_ASSERT_EQ(0, ft_strcmp("bar", null_prefixed_value));
    FT_ASSERT_EQ(ft_nullptr, adv_strjoin(ft_nullptr, ft_nullptr));
    cma_free(joined_value);
    cma_free(null_prefixed_value);
    return (1);
}

FT_TEST(test_adv_strjoin_allocation_failure,
    "adv_strjoin returns null when allocation fails")
{
    char *joined_value;

    cma_set_alloc_limit(1);
    joined_value = adv_strjoin("foo", "bar");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, joined_value);
    return (1);
}

FT_TEST(test_adv_substr_handles_bounds,
    "adv_substr slices ranges and returns empty when start exceeds length")
{
    char *middle_value;
    char *empty_value;

    middle_value = adv_substr("abcdef", 2, 3);
    empty_value = adv_substr("abc", 9, 5);
    FT_ASSERT(middle_value != ft_nullptr);
    FT_ASSERT(empty_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("cde", middle_value));
    FT_ASSERT_EQ(0, ft_strcmp("", empty_value));
    FT_ASSERT_EQ(ft_nullptr, adv_substr(ft_nullptr, 0, 2));
    cma_free(middle_value);
    cma_free(empty_value);
    return (1);
}

FT_TEST(test_adv_substr_clamps_requested_length,
    "adv_substr truncates requested length to available characters")
{
    char *substring_value;

    substring_value = adv_substr("abcd", 1, 40);
    FT_ASSERT(substring_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("bcd", substring_value));
    cma_free(substring_value);
    return (1);
}

FT_TEST(test_adv_substr_zero_requested_length,
    "adv_substr returns empty string when requested length is zero")
{
    char *substring_value;

    substring_value = adv_substr("abcd", 1, 0);
    FT_ASSERT(substring_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("", substring_value));
    cma_free(substring_value);
    return (1);
}

FT_TEST(test_adv_strtrim_trims_from_both_sides,
    "adv_strtrim removes set characters from both ends")
{
    char *trimmed_value;

    trimmed_value = adv_strtrim("***hello***", "*");
    FT_ASSERT(trimmed_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("hello", trimmed_value));
    FT_ASSERT_EQ(ft_nullptr, adv_strtrim(ft_nullptr, "*"));
    FT_ASSERT_EQ(ft_nullptr, adv_strtrim("hello", ft_nullptr));
    cma_free(trimmed_value);
    return (1);
}

FT_TEST(test_adv_strtrim_can_produce_empty_string,
    "adv_strtrim returns empty string when all characters are trimmed")
{
    char *trimmed_value;

    trimmed_value = adv_strtrim("xxxx", "x");
    FT_ASSERT(trimmed_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("", trimmed_value));
    cma_free(trimmed_value);
    return (1);
}

FT_TEST(test_adv_strtrim_empty_set_keeps_input,
    "adv_strtrim with empty set returns an unchanged copy")
{
    char *trimmed_value;

    trimmed_value = adv_strtrim("keep_me", "");
    FT_ASSERT(trimmed_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("keep_me", trimmed_value));
    cma_free(trimmed_value);
    return (1);
}

FT_TEST(test_adv_split_splits_non_empty_tokens,
    "adv_split returns only non-empty segments")
{
    char **split_result;

    split_result = adv_split(",alpha,,beta,", ',');
    FT_ASSERT(split_result != ft_nullptr);
    FT_ASSERT(split_result[0] != ft_nullptr);
    FT_ASSERT(split_result[1] != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("alpha", split_result[0]));
    FT_ASSERT_EQ(0, ft_strcmp("beta", split_result[1]));
    FT_ASSERT_EQ(ft_nullptr, split_result[2]);
    free_split_result(split_result);
    return (1);
}

FT_TEST(test_adv_split_rejects_null_input,
    "adv_split returns null for null input string")
{
    FT_ASSERT_EQ(ft_nullptr, adv_split(ft_nullptr, ','));
    return (1);
}

FT_TEST(test_adv_split_handles_all_delimiters_and_no_delimiter,
    "adv_split returns empty list for delimiters-only and one token when delimiter is absent")
{
    char **split_only_delimiters;
    char **split_without_delimiter;

    split_only_delimiters = adv_split(",,,", ',');
    FT_ASSERT(split_only_delimiters != ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, split_only_delimiters[0]);
    cma_free(split_only_delimiters);

    split_without_delimiter = adv_split("alpha", ',');
    FT_ASSERT(split_without_delimiter != ft_nullptr);
    FT_ASSERT(split_without_delimiter[0] != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("alpha", split_without_delimiter[0]));
    FT_ASSERT_EQ(ft_nullptr, split_without_delimiter[1]);
    free_split_result(split_without_delimiter);
    return (1);
}

FT_TEST(test_adv_split_null_delimiter_returns_whole_string,
    "adv_split with delimiter null character returns one token for non-empty strings")
{
    char **split_result;

    split_result = adv_split("alpha", '\0');
    FT_ASSERT(split_result != ft_nullptr);
    FT_ASSERT(split_result[0] != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("alpha", split_result[0]));
    FT_ASSERT_EQ(ft_nullptr, split_result[1]);
    free_split_result(split_result);
    return (1);
}

FT_TEST(test_adv_split_allocation_failure,
    "adv_split returns null when allocations fail")
{
    char **split_result;

    cma_set_alloc_limit(1);
    split_result = adv_split("a,b", ',');
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, split_result);
    return (1);
}

FT_TEST(test_adv_split_late_allocation_failure_releases_all_memory,
    "adv_split frees partial state when token allocation fails after result array allocation")
{
    char **split_result;
    ft_size_t bytes_before;
    ft_size_t bytes_after;
    const char *long_token;

    long_token = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    split_result = adv_split("warm,up", ',');
    free_split_result(split_result);
    bytes_before = advanced_current_allocated_bytes();
    cma_set_alloc_limit(32);
    split_result = adv_split(long_token, ',');
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, split_result);
    bytes_after = advanced_current_allocated_bytes();
    FT_ASSERT_EQ(bytes_before, bytes_after);
    return (1);
}

FT_TEST(test_adv_span_to_string_respects_length,
    "adv_span_to_string creates ft_string from a byte span")
{
    const char raw_buffer[5] = {'a', 'b', '\0', 'c', 'd'};
    ft_string *result_string;
    const char *character_pointer;

    result_string = adv_span_to_string(raw_buffer, 5);
    FT_ASSERT(result_string != ft_nullptr);
    FT_ASSERT_EQ(5, result_string->size());
    character_pointer = result_string->at(0);
    FT_ASSERT(character_pointer != ft_nullptr);
    FT_ASSERT_EQ('a', *character_pointer);
    character_pointer = result_string->at(1);
    FT_ASSERT(character_pointer != ft_nullptr);
    FT_ASSERT_EQ('b', *character_pointer);
    character_pointer = result_string->at(2);
    FT_ASSERT(character_pointer != ft_nullptr);
    FT_ASSERT_EQ('\0', *character_pointer);
    character_pointer = result_string->at(3);
    FT_ASSERT(character_pointer != ft_nullptr);
    FT_ASSERT_EQ('c', *character_pointer);
    character_pointer = result_string->at(4);
    FT_ASSERT(character_pointer != ft_nullptr);
    FT_ASSERT_EQ('d', *character_pointer);
    delete result_string;
    FT_ASSERT_EQ(ft_nullptr, adv_span_to_string(ft_nullptr, 1));
    return (1);
}

FT_TEST(test_adv_span_to_string_zero_length_null_buffer,
    "adv_span_to_string accepts null buffer when length is zero")
{
    ft_string *result_string;

    result_string = adv_span_to_string(ft_nullptr, 0);
    FT_ASSERT(result_string != ft_nullptr);
    FT_ASSERT_EQ(0, result_string->size());
    delete result_string;
    return (1);
}

FT_TEST(test_adv_span_to_string_allocation_failure,
    "adv_span_to_string returns null when allocations fail")
{
    ft_string *result_string;

    cma_set_alloc_limit(1);
    result_string = adv_span_to_string("abc", 3);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, result_string);
    return (1);
}

FT_TEST(test_adv_span_to_string_late_append_failure_releases_memory,
    "adv_span_to_string frees temporary state when append fails after initialization")
{
    ft_string *result_string;
    ft_size_t bytes_before;
    ft_size_t bytes_after;
    const char *long_buffer;

    long_buffer = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    result_string = adv_span_to_string("warmup", 6);
    FT_ASSERT(result_string != ft_nullptr);
    delete result_string;
    bytes_before = advanced_current_allocated_bytes();
    cma_set_alloc_limit(32);
    result_string = adv_span_to_string(long_buffer, 62);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, result_string);
    bytes_after = advanced_current_allocated_bytes();
    FT_ASSERT_EQ(bytes_before, bytes_after);
    return (1);
}

FT_TEST(test_adv_to_string_and_locale_casefold,
    "adv_to_string and adv_locale_casefold return initialized ft_string objects")
{
    ft_string *number_string;
    ft_string *folded_string;

    number_string = adv_to_string(-77);
    FT_ASSERT(number_string != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("-77", number_string->c_str()));
    delete number_string;

    folded_string = adv_locale_casefold("MiXeD", "C");
    FT_ASSERT(folded_string != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("mixed", folded_string->c_str()));
    delete folded_string;

    FT_ASSERT_EQ(ft_nullptr, adv_locale_casefold(ft_nullptr, "C"));
    FT_ASSERT_EQ(ft_nullptr, adv_locale_casefold("MiXeD", "invalid_locale_name"));
    return (1);
}
