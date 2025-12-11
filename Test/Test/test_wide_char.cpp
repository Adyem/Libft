#include "../../Libft/libft.hpp"
#include "../../Libft/libft_utf8.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_wstrlen_counts_length, "ft_wstrlen counts wide characters")
{
    const wchar_t *wide_text;

    wide_text = L"hello";
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(static_cast<size_t>(5), ft_wstrlen(wide_text));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_wstrlen_null_pointer_sets_errno, "ft_wstrlen validates null pointers")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(static_cast<size_t>(0), ft_wstrlen(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_utf16_to_utf8_ascii_roundtrip, "ft_utf16_to_utf8 converts ascii input")
{
    char16_t input[3];
    ft_string utf8_result;

    input[0] = static_cast<char16_t>('A');
    input[1] = static_cast<char16_t>('B');
    input[2] = static_cast<char16_t>(0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    utf8_result = ft_utf16_to_utf8(input, 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, utf8_result.get_error());
    FT_ASSERT(utf8_result == "AB");
    return (1);
}

FT_TEST(test_utf16_to_utf8_surrogate_pair, "ft_utf16_to_utf8 handles surrogate pairs")
{
    char16_t input[3];
    ft_string utf8_result;
    char expected[5];

    input[0] = static_cast<char16_t>(0xD83D);
    input[1] = static_cast<char16_t>(0xDE00);
    input[2] = static_cast<char16_t>(0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    utf8_result = ft_utf16_to_utf8(input, 2);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, utf8_result.get_error());
    expected[0] = static_cast<char>(0xF0);
    expected[1] = static_cast<char>(0x9F);
    expected[2] = static_cast<char>(0x98);
    expected[3] = static_cast<char>(0x80);
    expected[4] = '\0';
    FT_ASSERT_EQ(static_cast<size_t>(4), ft_strlen_size_t(utf8_result.c_str()));
    FT_ASSERT_EQ(0, ft_strcmp(expected, utf8_result.c_str()));
    return (1);
}

FT_TEST(test_utf16_to_utf8_invalid_surrogate_reports_error, "ft_utf16_to_utf8 rejects orphaned high surrogate")
{
    char16_t input[2];
    ft_string utf8_result;

    input[0] = static_cast<char16_t>(0xD83D);
    input[1] = static_cast<char16_t>(0);
    ft_errno = FT_ERR_SUCCESSS;
    utf8_result = ft_utf16_to_utf8(input, 2);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, utf8_result.get_error());
    return (1);
}

FT_TEST(test_utf32_to_utf8_mixed_sequence, "ft_utf32_to_utf8 encodes code points")
{
    char32_t input[4];
    ft_string utf8_result;
    char expected[7];

    input[0] = static_cast<char32_t>('A');
    input[1] = static_cast<char32_t>(0x1F642);
    input[2] = static_cast<char32_t>('B');
    input[3] = static_cast<char32_t>(0);
    ft_errno = FT_ERR_SUCCESSS;
    utf8_result = ft_utf32_to_utf8(input, 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, utf8_result.get_error());
    expected[0] = 'A';
    expected[1] = static_cast<char>(0xF0);
    expected[2] = static_cast<char>(0x9F);
    expected[3] = static_cast<char>(0x99);
    expected[4] = static_cast<char>(0x82);
    expected[5] = 'B';
    expected[6] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(expected, utf8_result.c_str()));
    return (1);
}

FT_TEST(test_utf8_to_utf16_round_trip, "ft_utf8_to_utf16 converts and preserves code points")
{
    char utf8_input[7];
    char16_t *utf16_output;
    size_t utf16_length;
    ft_string round_trip;

    utf8_input[0] = 'A';
    utf8_input[1] = static_cast<char>(0xF0);
    utf8_input[2] = static_cast<char>(0x9F);
    utf8_input[3] = static_cast<char>(0x98);
    utf8_input[4] = static_cast<char>(0x80);
    utf8_input[5] = 'B';
    utf8_input[6] = '\0';
    utf16_output = ft_utf8_to_utf16(utf8_input, 0, &utf16_length);
    FT_ASSERT(utf16_output != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(static_cast<size_t>(4), utf16_length);
    FT_ASSERT_EQ(static_cast<char16_t>('A'), utf16_output[0]);
    FT_ASSERT_EQ(static_cast<char16_t>(0xD83D), utf16_output[1]);
    FT_ASSERT_EQ(static_cast<char16_t>(0xDE00), utf16_output[2]);
    FT_ASSERT_EQ(static_cast<char16_t>('B'), utf16_output[3]);
    round_trip = ft_utf16_to_utf8(utf16_output, utf16_length);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, round_trip.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(utf8_input, round_trip.c_str()));
    cma_free(utf16_output);
    return (1);
}

FT_TEST(test_utf8_to_utf16_invalid_sequence_sets_errno, "ft_utf8_to_utf16 reports invalid byte sequences")
{
    char invalid_utf8[3];
    char16_t *utf16_output;

    invalid_utf8[0] = static_cast<char>(0xE0);
    invalid_utf8[1] = static_cast<char>(0x80);
    invalid_utf8[2] = '\0';
    ft_errno = FT_ERR_SUCCESSS;
    utf16_output = ft_utf8_to_utf16(invalid_utf8, 2, ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, utf16_output);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_utf8_to_utf32_invalid_sequence_resets_errno, "ft_utf8_to_utf32 clears errno after handling invalid input")
{
    char invalid_utf8[2];
    char32_t *utf32_output;
    size_t utf32_length;

    invalid_utf8[0] = static_cast<char>(0xC2);
    invalid_utf8[1] = '\0';
    ft_errno = FT_ERR_SUCCESSS;
    utf32_output = ft_utf8_to_utf32(invalid_utf8, 0, &utf32_length);
    FT_ASSERT_EQ(ft_nullptr, utf32_output);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    utf32_output = ft_utf8_to_utf32("OK", 0, &utf32_length);
    FT_ASSERT(utf32_output != ft_nullptr);
    FT_ASSERT_EQ(static_cast<size_t>(2), utf32_length);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    cma_free(utf32_output);
    return (1);
}

FT_TEST(test_utf8_to_utf32_round_trip, "ft_utf8_to_utf32 decodes multi-byte sequences")
{
    char utf8_input[8];
    char32_t *utf32_output;
    size_t utf32_length;
    size_t index;

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
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(static_cast<size_t>(2), utf32_length);
    index = 0;
    while (index < utf32_length)
    {
        if (index == 0)
            FT_ASSERT_EQ(static_cast<char32_t>(0x20AC), utf32_output[index]);
        else if (index == 1)
            FT_ASSERT_EQ(static_cast<char32_t>(0x1F600), utf32_output[index]);
        index++;
    }
    cma_free(utf32_output);
    return (1);
}

FT_TEST(test_utf8_to_utf32_invalid_sequence_sets_errno, "ft_utf8_to_utf32 mirrors decoder errors")
{
    char invalid_utf8[2];
    char32_t *utf32_output;

    invalid_utf8[0] = static_cast<char>(0xC0);
    invalid_utf8[1] = '\0';
    ft_errno = FT_ERR_SUCCESSS;
    utf32_output = ft_utf8_to_utf32(invalid_utf8, 1, ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, utf32_output);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
