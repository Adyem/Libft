#include "../../Libft/libft.hpp"
#include "../../Libft/libft_utf8.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_utf8_count_ascii, "ft_utf8_count counts ascii code points")
{
    size_t code_point_count;

    code_point_count = 0;
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_count("hello", &code_point_count));
    FT_ASSERT_EQ(static_cast<size_t>(5), code_point_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_utf8_next_multibyte, "ft_utf8_next decodes multibyte sequences")
{
    const char *utf8_string;
    size_t string_length;
    size_t index_pointer;
    uint32_t code_point_value;
    size_t sequence_length;

    utf8_string = "héllo";
    string_length = ft_strlen_size_t(utf8_string);
    index_pointer = 0;
    code_point_value = 0;
    sequence_length = 0;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_next(utf8_string, string_length,
            &index_pointer, &code_point_value, &sequence_length));
    FT_ASSERT_EQ(static_cast<uint32_t>('h'), code_point_value);
    FT_ASSERT_EQ(static_cast<size_t>(1), sequence_length);
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_next(utf8_string, string_length,
            &index_pointer, &code_point_value, &sequence_length));
    FT_ASSERT_EQ(static_cast<uint32_t>(0xE9), code_point_value);
    FT_ASSERT_EQ(static_cast<size_t>(2), sequence_length);
    return (1);
}

FT_TEST(test_utf8_next_four_byte_sequence, "ft_utf8_next decodes emoji code points")
{
    const char *utf8_string;
    size_t string_length;
    size_t index_pointer;
    uint32_t code_point_value;
    size_t sequence_length;

    utf8_string = "A" "\xF0\x9F\x98\x80" "B";
    string_length = ft_strlen_size_t(utf8_string);
    index_pointer = 0;
    code_point_value = 0;
    sequence_length = 0;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_next(utf8_string, string_length,
            &index_pointer, &code_point_value, &sequence_length));
    FT_ASSERT_EQ(static_cast<uint32_t>('A'), code_point_value);
    FT_ASSERT_EQ(static_cast<size_t>(1), sequence_length);
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_next(utf8_string, string_length,
            &index_pointer, &code_point_value, &sequence_length));
    FT_ASSERT_EQ(static_cast<uint32_t>(0x1F600), code_point_value);
    FT_ASSERT_EQ(static_cast<size_t>(4), sequence_length);
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_next(utf8_string, string_length,
            &index_pointer, &code_point_value, &sequence_length));
    FT_ASSERT_EQ(static_cast<uint32_t>('B'), code_point_value);
    FT_ASSERT_EQ(static_cast<size_t>(1), sequence_length);
    return (1);
}

FT_TEST(test_utf8_next_invalid_sequence_sets_errno, "ft_utf8_next reports invalid byte sequences")
{
    char invalid_sequence[3];
    size_t string_length;
    size_t index_pointer;
    uint32_t code_point_value;

    invalid_sequence[0] = static_cast<char>(0xC0);
    invalid_sequence[1] = static_cast<char>(0xAF);
    invalid_sequence[2] = '\0';
    string_length = static_cast<size_t>(2);
    index_pointer = 0;
    code_point_value = 0;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_next(invalid_sequence, string_length,
            &index_pointer, &code_point_value, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_utf8_next_recovers_after_invalid_sequence, "ft_utf8_next clears errno after failure")
{
    char invalid_sequence[3];
    const char *valid_string;
    size_t string_length;
    size_t index_pointer;
    uint32_t code_point_value;
    size_t sequence_length;

    invalid_sequence[0] = static_cast<char>(0xC1);
    invalid_sequence[1] = static_cast<char>(0xBF);
    invalid_sequence[2] = '\0';
    string_length = static_cast<size_t>(2);
    index_pointer = 0;
    code_point_value = 0;
    sequence_length = 0;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_next(invalid_sequence, string_length,
            &index_pointer, &code_point_value, &sequence_length));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    valid_string = "A";
    string_length = ft_strlen_size_t(valid_string);
    index_pointer = 0;
    code_point_value = 0;
    sequence_length = 0;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_next(valid_string, string_length,
            &index_pointer, &code_point_value, &sequence_length));
    FT_ASSERT_EQ(static_cast<uint32_t>('A'), code_point_value);
    FT_ASSERT_EQ(static_cast<size_t>(1), sequence_length);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_utf8_count_null_pointer_sets_errno, "ft_utf8_count rejects null inputs and recovers")
{
    size_t code_point_count;

    code_point_count = 1;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_count(ft_nullptr, &code_point_count));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(static_cast<size_t>(1), code_point_count);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_count("ok", &code_point_count));
    FT_ASSERT_EQ(static_cast<size_t>(2), code_point_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

static uint32_t ft_utf8_test_invalid_hook(uint32_t code_point)
{
    (void)code_point;
    return (0x110000);
}

FT_TEST(test_utf8_transform_alloc_lowercase, "ft_utf8_transform_alloc applies ascii lower hook")
{
    char *transformed_string;

    transformed_string = ft_nullptr;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_transform_alloc("TeSt",
            &transformed_string, ft_utf8_case_ascii_lower));
    FT_ASSERT(transformed_string != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("test", transformed_string));
    cma_free(transformed_string);
    return (1);
}

FT_TEST(test_utf8_transform_alloc_uppercase, "ft_utf8_transform_alloc applies ascii upper hook")
{
    char *transformed_string;

    transformed_string = ft_nullptr;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_transform_alloc("emoji: " "\xF0\x9F\x98\x80", &transformed_string,
            ft_utf8_case_ascii_upper));
    FT_ASSERT(transformed_string != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("EMOJI: " "\xF0\x9F\x98\x80", transformed_string));
    cma_free(transformed_string);
    return (1);
}

FT_TEST(test_utf8_grapheme_duplicate_combining_mark, "ft_utf8_duplicate_grapheme copies composed characters")
{
    const char *grapheme_string;
    size_t string_length;
    size_t index_pointer;
    char *grapheme_copy;

    grapheme_string = "e\xCC\x81o";
    string_length = ft_strlen_size_t(grapheme_string);
    index_pointer = 0;
    grapheme_copy = ft_nullptr;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_duplicate_grapheme(grapheme_string,
            string_length, &index_pointer, &grapheme_copy));
    FT_ASSERT(grapheme_copy != ft_nullptr);
    FT_ASSERT_EQ(static_cast<size_t>(3), ft_strlen_size_t(grapheme_copy));
    FT_ASSERT_EQ(0, ft_strcmp("e\xCC\x81", grapheme_copy));
    cma_free(grapheme_copy);
    FT_ASSERT_EQ(static_cast<size_t>(3), index_pointer);
    return (1);
}

FT_TEST(test_utf8_count_invalid_sets_errno, "ft_utf8_count mirrors invalid bytes")
{
    char invalid_sequence[3];
    size_t code_point_count;

    invalid_sequence[0] = static_cast<char>(0xE0);
    invalid_sequence[1] = static_cast<char>(0x80);
    invalid_sequence[2] = '\0';
    code_point_count = 0;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_count(invalid_sequence, &code_point_count));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_utf8_encode_emoji, "ft_utf8_encode writes four byte sequence")
{
    char buffer[5];
    size_t encoded_length;

    buffer[0] = '\0';
    buffer[1] = '\0';
    buffer[2] = '\0';
    buffer[3] = '\0';
    buffer[4] = '\0';
    encoded_length = 0;
    FT_ASSERT_EQ(FT_SUCCESS, ft_utf8_encode(0x1F642, buffer, sizeof(buffer),
            &encoded_length));
    FT_ASSERT_EQ(static_cast<size_t>(4), encoded_length);
    FT_ASSERT_EQ(static_cast<char>(0xF0), buffer[0]);
    FT_ASSERT_EQ(static_cast<char>(0x9F), buffer[1]);
    FT_ASSERT_EQ(static_cast<char>(0x99), buffer[2]);
    FT_ASSERT_EQ(static_cast<char>(0x82), buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    return (1);
}

FT_TEST(test_utf8_encode_reports_small_buffer, "ft_utf8_encode mirrors range errors")
{
    char buffer[3];

    buffer[0] = '\0';
    buffer[1] = '\0';
    buffer[2] = '\0';
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_encode(0x20AC, buffer, sizeof(buffer), ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_utf8_next_rejects_surrogate, "ft_utf8_next rejects utf16 surrogate code points")
{
    const char *invalid_string;
    size_t string_length;
    size_t index_pointer;
    uint32_t code_point_value;

    invalid_string = "\xED\xA0\x80";
    string_length = static_cast<size_t>(3);
    index_pointer = 0;
    code_point_value = 0;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_next(invalid_string, string_length,
            &index_pointer, &code_point_value, ft_nullptr));
    FT_ASSERT_EQ(static_cast<size_t>(0), index_pointer);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_utf8_next_detects_truncated_sequence, "ft_utf8_next rejects truncated multibyte sequences")
{
    const char *invalid_string;
    size_t string_length;
    size_t index_pointer;
    uint32_t code_point_value;

    invalid_string = "\xE2\x82";
    string_length = static_cast<size_t>(2);
    index_pointer = 0;
    code_point_value = 0;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_next(invalid_string, string_length,
            &index_pointer, &code_point_value, ft_nullptr));
    FT_ASSERT_EQ(static_cast<size_t>(0), index_pointer);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_utf8_next_null_pointer_guard, "ft_utf8_next validates pointers before decoding")
{
    size_t index_pointer;
    uint32_t code_point_value;

    index_pointer = 0;
    code_point_value = 0;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_next(ft_nullptr, static_cast<size_t>(0),
            &index_pointer, &code_point_value, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_utf8_encode_rejects_surrogate, "ft_utf8_encode rejects utf16 surrogate values")
{
    char buffer[4];

    buffer[0] = '\0';
    buffer[1] = '\0';
    buffer[2] = '\0';
    buffer[3] = '\0';
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_encode(0xD800, buffer, sizeof(buffer), ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_utf8_transform_alloc_propagates_hook_error, "ft_utf8_transform_alloc surfaces hook produced invalid code point")
{
    char *transformed_string;

    transformed_string = ft_nullptr;
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_utf8_transform_alloc("abc", &transformed_string,
            ft_utf8_test_invalid_hook));
    FT_ASSERT_EQ(ft_nullptr, transformed_string);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
