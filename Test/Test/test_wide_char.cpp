#include "../test_internal.hpp"
#include "../../Advanced/advanced.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

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

FT_TEST(test_wstrlen_counts_length, "ft_wstrlen counts wide characters")
{
    const wchar_t *wide_text;

    wide_text = L"hello";
    FT_ASSERT_EQ(static_cast<ft_size_t>(5), ft_wstrlen(wide_text));
    return (1);
}

FT_TEST(test_wstrlen_null_pointer_returns_zero, "ft_wstrlen returns zero for null input")
{
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), ft_wstrlen(ft_nullptr));
    return (1);
}

FT_TEST(test_adv_utf16_to_utf8_ascii_roundtrip, "adv_utf16_to_utf8 converts ascii input")
{
    char16_t input[3];
    ft_string *utf8_result;

    input[0] = static_cast<char16_t>('A');
    input[1] = static_cast<char16_t>('B');
    input[2] = static_cast<char16_t>(0);
    utf8_result = adv_utf16_to_utf8(input, 0);
    FT_ASSERT(utf8_result != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("AB", utf8_result->c_str()));
    delete utf8_result;
    return (1);
}

FT_TEST(test_adv_utf16_to_utf8_surrogate_pair, "adv_utf16_to_utf8 handles surrogate pairs")
{
    char16_t input[3];
    ft_string *utf8_result;
    char expected[5];

    input[0] = static_cast<char16_t>(0xD83D);
    input[1] = static_cast<char16_t>(0xDE00);
    input[2] = static_cast<char16_t>(0);
    utf8_result = adv_utf16_to_utf8(input, 2);
    FT_ASSERT(utf8_result != ft_nullptr);
    expected[0] = static_cast<char>(0xF0);
    expected[1] = static_cast<char>(0x9F);
    expected[2] = static_cast<char>(0x98);
    expected[3] = static_cast<char>(0x80);
    expected[4] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(expected, utf8_result->c_str()));
    delete utf8_result;
    return (1);
}

FT_TEST(test_adv_utf16_to_utf8_invalid_surrogate_returns_null, "adv_utf16_to_utf8 rejects orphaned high surrogate")
{
    char16_t input[2];
    ft_string *utf8_result;

    input[0] = static_cast<char16_t>(0xD83D);
    input[1] = static_cast<char16_t>(0);
    utf8_result = adv_utf16_to_utf8(input, 2);
    FT_ASSERT_EQ(ft_nullptr, utf8_result);
    return (1);
}

FT_TEST(test_adv_utf16_to_utf8_null_pointer_with_non_zero_length,
    "adv_utf16_to_utf8 rejects null input when length is non-zero")
{
    FT_ASSERT_EQ(ft_nullptr, adv_utf16_to_utf8(ft_nullptr, 1));
    return (1);
}

FT_TEST(test_adv_utf16_to_utf8_null_pointer_with_zero_length,
    "adv_utf16_to_utf8 returns an empty string for null input with zero length")
{
    ft_string *utf8_result;

    utf8_result = adv_utf16_to_utf8(ft_nullptr, 0);
    FT_ASSERT(utf8_result != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("", utf8_result->c_str()));
    delete utf8_result;
    return (1);
}

FT_TEST(test_adv_utf16_to_utf8_allocation_failure,
    "adv_utf16_to_utf8 returns null when allocations fail")
{
    char16_t input[2];
    ft_string *utf8_result;

    input[0] = static_cast<char16_t>('A');
    input[1] = static_cast<char16_t>(0);
    cma_set_alloc_limit(1);
    utf8_result = adv_utf16_to_utf8(input, 1);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, utf8_result);
    return (1);
}

FT_TEST(test_adv_utf32_to_utf8_mixed_sequence, "adv_utf32_to_utf8 encodes code points")
{
    char32_t input[4];
    ft_string *utf8_result;
    char expected[7];

    input[0] = static_cast<char32_t>('A');
    input[1] = static_cast<char32_t>(0x1F642);
    input[2] = static_cast<char32_t>('B');
    input[3] = static_cast<char32_t>(0);
    utf8_result = adv_utf32_to_utf8(input, 0);
    FT_ASSERT(utf8_result != ft_nullptr);
    expected[0] = 'A';
    expected[1] = static_cast<char>(0xF0);
    expected[2] = static_cast<char>(0x9F);
    expected[3] = static_cast<char>(0x99);
    expected[4] = static_cast<char>(0x82);
    expected[5] = 'B';
    expected[6] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(expected, utf8_result->c_str()));
    delete utf8_result;
    return (1);
}

FT_TEST(test_adv_utf32_to_utf8_rejects_invalid_code_points,
    "adv_utf32_to_utf8 rejects surrogate and out-of-range code points")
{
    char32_t surrogate_input[2];
    char32_t out_of_range_input[2];

    surrogate_input[0] = static_cast<char32_t>(0xD800);
    surrogate_input[1] = static_cast<char32_t>(0);
    out_of_range_input[0] = static_cast<char32_t>(0x110000);
    out_of_range_input[1] = static_cast<char32_t>(0);
    FT_ASSERT_EQ(ft_nullptr, adv_utf32_to_utf8(surrogate_input, 1));
    FT_ASSERT_EQ(ft_nullptr, adv_utf32_to_utf8(out_of_range_input, 1));
    FT_ASSERT_EQ(ft_nullptr, adv_utf32_to_utf8(ft_nullptr, 1));
    return (1);
}

FT_TEST(test_adv_utf32_to_utf8_null_pointer_with_zero_length,
    "adv_utf32_to_utf8 returns an empty string for null input with zero length")
{
    ft_string *utf8_result;

    utf8_result = adv_utf32_to_utf8(ft_nullptr, 0);
    FT_ASSERT(utf8_result != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("", utf8_result->c_str()));
    delete utf8_result;
    return (1);
}

FT_TEST(test_adv_utf32_to_utf8_allocation_failure,
    "adv_utf32_to_utf8 returns null when allocations fail")
{
    char32_t input[2];
    ft_string *utf8_result;

    input[0] = static_cast<char32_t>('A');
    input[1] = static_cast<char32_t>(0);
    cma_set_alloc_limit(1);
    utf8_result = adv_utf32_to_utf8(input, 1);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, utf8_result);
    return (1);
}

FT_TEST(test_adv_utf16_to_utf8_late_growth_failure_releases_memory,
    "adv_utf16_to_utf8 cleans internal allocations when append growth fails later")
{
    char16_t input[129];
    ft_string *utf8_result;
    ft_size_t index;
    ft_size_t bytes_before;
    ft_size_t bytes_after;

    index = 0;
    while (index < 128)
    {
        input[index] = static_cast<char16_t>('A');
        index += 1;
    }
    input[128] = static_cast<char16_t>(0);
    utf8_result = adv_utf16_to_utf8(input, 8);
    FT_ASSERT(utf8_result != ft_nullptr);
    delete utf8_result;
    bytes_before = advanced_current_allocated_bytes();
    cma_set_alloc_limit(32);
    utf8_result = adv_utf16_to_utf8(input, 128);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, utf8_result);
    bytes_after = advanced_current_allocated_bytes();
    FT_ASSERT_EQ(bytes_before, bytes_after);
    return (1);
}

FT_TEST(test_utf8_to_utf16_round_trip, "ft_utf8_to_utf16 converts and preserves code points")
{
    char utf8_input[7];
    char16_t *utf16_output;
    ft_size_t utf16_length;
    ft_string *round_trip;

    utf8_input[0] = 'A';
    utf8_input[1] = static_cast<char>(0xF0);
    utf8_input[2] = static_cast<char>(0x9F);
    utf8_input[3] = static_cast<char>(0x98);
    utf8_input[4] = static_cast<char>(0x80);
    utf8_input[5] = 'B';
    utf8_input[6] = '\0';
    utf16_output = ft_utf8_to_utf16(utf8_input, 0, &utf16_length);
    FT_ASSERT(utf16_output != ft_nullptr);
    FT_ASSERT_EQ(static_cast<ft_size_t>(4), utf16_length);
    FT_ASSERT_EQ(static_cast<char16_t>('A'), utf16_output[0]);
    FT_ASSERT_EQ(static_cast<char16_t>(0xD83D), utf16_output[1]);
    FT_ASSERT_EQ(static_cast<char16_t>(0xDE00), utf16_output[2]);
    FT_ASSERT_EQ(static_cast<char16_t>('B'), utf16_output[3]);
    round_trip = adv_utf16_to_utf8(utf16_output, utf16_length);
    FT_ASSERT(round_trip != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(utf8_input, round_trip->c_str()));
    delete round_trip;
    cma_free(utf16_output);
    return (1);
}

FT_TEST(test_utf8_to_utf16_invalid_sequence_returns_null, "ft_utf8_to_utf16 reports invalid byte sequences")
{
    char invalid_utf8[3];
    char16_t *utf16_output;

    invalid_utf8[0] = static_cast<char>(0xE0);
    invalid_utf8[1] = static_cast<char>(0x80);
    invalid_utf8[2] = '\0';
    utf16_output = ft_utf8_to_utf16(invalid_utf8, 2, ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, utf16_output);
    return (1);
}

FT_TEST(test_utf8_to_utf16_null_input_with_non_zero_length,
    "ft_utf8_to_utf16 rejects null input for non-zero length")
{
    FT_ASSERT_EQ(ft_nullptr, ft_utf8_to_utf16(ft_nullptr, 1, ft_nullptr));
    return (1);
}

FT_TEST(test_utf8_to_utf16_null_input_with_zero_length,
    "ft_utf8_to_utf16 returns an empty allocation for null input with zero length")
{
    char16_t *utf16_output;
    ft_size_t utf16_length;

    utf16_output = ft_utf8_to_utf16(ft_nullptr, 0, &utf16_length);
    FT_ASSERT(utf16_output != ft_nullptr);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), utf16_length);
    FT_ASSERT_EQ(static_cast<char16_t>(0), utf16_output[0]);
    cma_free(utf16_output);
    return (1);
}

FT_TEST(test_utf8_to_utf32_round_trip, "ft_utf8_to_utf32 decodes multi-byte sequences")
{
    char utf8_input[8];
    char32_t *utf32_output;
    ft_size_t utf32_length;

    utf8_input[0] = static_cast<char>(0xE2);
    utf8_input[1] = static_cast<char>(0x82);
    utf8_input[2] = static_cast<char>(0xAC);
    utf8_input[3] = static_cast<char>(0xF0);
    utf8_input[4] = static_cast<char>(0x9F);
    utf8_input[5] = static_cast<char>(0x98);
    utf8_input[6] = static_cast<char>(0x80);
    utf8_input[7] = '\0';
    utf32_output = ft_utf8_to_utf32(utf8_input, 0, &utf32_length);
    FT_ASSERT(utf32_output != ft_nullptr);
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), utf32_length);
    FT_ASSERT_EQ(static_cast<char32_t>(0x20AC), utf32_output[0]);
    FT_ASSERT_EQ(static_cast<char32_t>(0x1F600), utf32_output[1]);
    cma_free(utf32_output);
    return (1);
}

FT_TEST(test_utf8_to_utf32_invalid_sequence_returns_null, "ft_utf8_to_utf32 mirrors decoder errors")
{
    char invalid_utf8[2];
    char32_t *utf32_output;

    invalid_utf8[0] = static_cast<char>(0xC0);
    invalid_utf8[1] = '\0';
    utf32_output = ft_utf8_to_utf32(invalid_utf8, 1, ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, utf32_output);
    return (1);
}

FT_TEST(test_utf8_to_utf32_null_input_with_non_zero_length,
    "ft_utf8_to_utf32 rejects null input for non-zero length")
{
    FT_ASSERT_EQ(ft_nullptr, ft_utf8_to_utf32(ft_nullptr, 1, ft_nullptr));
    return (1);
}

FT_TEST(test_utf8_to_utf32_null_input_with_zero_length,
    "ft_utf8_to_utf32 returns an empty allocation for null input with zero length")
{
    char32_t *utf32_output;
    ft_size_t utf32_length;

    utf32_output = ft_utf8_to_utf32(ft_nullptr, 0, &utf32_length);
    FT_ASSERT(utf32_output != ft_nullptr);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), utf32_length);
    FT_ASSERT_EQ(static_cast<char32_t>(0), utf32_output[0]);
    cma_free(utf32_output);
    return (1);
}
